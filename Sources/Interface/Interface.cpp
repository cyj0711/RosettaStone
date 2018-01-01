/*************************************************************************
> File Name: Interface.cpp
> Project Name: Hearthstone++
> Author: Young-Joong Kim
> Purpose: Interface for Hearthstone Game Agent
> Created Time: 2017/10/24
> Copyright (c) 2017, Young-Joong Kim
*************************************************************************/
#include <Commons/Constants.h>
#include <Enums/EnumsToString.h>
#include <Interface/Interface.h>

namespace Hearthstonepp
{
	GameInterface::GameInterface(GameAgent& agent, std::ostream& output, std::istream& input) :
		m_agent(agent), m_bufferCapacity(agent.GetBufferCapacity()),
		ostream(output), istream(input)
	{
		m_buffer = new BYTE[m_bufferCapacity];
	}

	GameResult GameInterface::StartGame()
	{
		GameResult result;
		// pass as reference
		std::thread at = m_agent.StartAgent(result);

		while (true)
		{
			const int msg = HandleMessage();
			if (msg == HANDLE_STOP)
			{
				break;
			}
		}

		// join agent thread
		at.join();

		return result;
	}

	const int GameInterface::HandleMessage()
	{
		m_agent.ReadBuffer(m_buffer, m_bufferCapacity);
		
		if (m_handler.find(m_buffer[0]) != m_handler.end())
		{
			// find from handler table and call it
			m_handler[m_buffer[0]](*this);
		}

		if (m_buffer[0] == static_cast<BYTE>(Step::FINAL_GAMEOVER))
		{
			return HANDLE_STOP;
		}

		return HANDLE_CONTINUE;
	}

	std::ostream& GameInterface::LogWriter(std::string& name)
	{
		ostream << "[*] " << name << " : ";
		return ostream;
	}

	template <std::size_t SIZE>
	void GameInterface::ShowMenus(std::array<std::string, SIZE> menus)
	{
		for (auto& menu : menus)
		{
			ostream << menu << std::endl;
		}
	}

	void GameInterface::ShowCards(Card** cards, int size)
	{
		for (int i = 0; i < size; ++i)
		{
			std::string type = ConverterFromCardTypeToString.at(cards[i]->GetCardType());
			ostream << '[' << cards[i]->GetName() << '(' << type << " / " << cards[i]->GetCost() << ")] ";
			if (cards[i]->GetCardType() == CardType::MINION)
			{
				ostream << "(ATK " << cards[i]->GetAttack() << "/HP " << cards[i]->GetHealth() << ")";
			}
			ostream << std::endl;
		}
	}

	void GameInterface::BriefGame()
	{
		GameBrief* data = reinterpret_cast<GameBrief*>(m_buffer);

		LogWriter(m_users[data->currentUser]) << "Game Briefing" << std::endl;

		ostream << m_users[data->oppositeUser] 
			<< " - Hero " << data->oppositeHero->GetName()
			<< ", Health " << data->oppositeHero->GetHealth()
			<< ", Mana " << static_cast<int>(data->oppositeMana)
			<< ", Hand " << static_cast<int>(data->numOppositeHand) 
			<< std::endl;

		ostream << m_users[data->oppositeUser] << " Field" << std::endl;
		ShowCards(data->oppositeField, data->numOppositeField);

		ostream << m_users[data->currentUser]
			<< " - Hero " << data->currentHero->GetName()
			<< ", Health " << data->currentHero->GetHealth()
			<< ", Mana " << static_cast<int>(data->currentMana)
			<< ", Hand " << static_cast<int>(data->numCurrentHand)
			<< std::endl;

		ostream << m_users[data->currentUser] << " Field" << std::endl;
		ShowCards(data->currentField, data->numCurrentField);

		ostream << m_users[data->currentUser] << " Hand" << std::endl;
		ShowCards(data->currentHand, data->numCurrentHand);
	}

	void GameInterface::OverDraw()
	{
		OverDrawStructure* data = reinterpret_cast<OverDrawStructure*>(m_buffer);

		LogWriter(m_users[data->userID]) << "Over draw " << static_cast<int>(data->numOver) << " cards" << std::endl;
		ShowCards(data->cards, data->numOver);
	}

	void GameInterface::ExhaustDeck()
	{
		ExhaustDeckStructure* data = reinterpret_cast<ExhaustDeckStructure*>(m_buffer);

		LogWriter(m_users[data->userID]) << "Deck exhausted over " << static_cast<int>(data->numOver) << std::endl;
	}

	void GameInterface::ModifiedMana()
	{
		ModifyManaStructure* data = reinterpret_cast<ModifyManaStructure*>(m_buffer);

		LogWriter(m_users[data->userID]) << "Mana is modified to " << static_cast<int>(data->mana) << std::endl;
	}

	void GameInterface::ModifiedHealth()
	{
		ModifyHealthStructure* data = reinterpret_cast<ModifyHealthStructure*>(m_buffer);

		LogWriter(m_users[data->userID]) 
			<< "Health of "
			<< (data->card->GetCardType() == CardType::MINION ? "Minion " : "Hero ")
			<< data->card->GetName()
			<< " is modified to " << static_cast<int>(data->card->GetHealth())
			<< std::endl;
	}

	void GameInterface::ExhaustMinion()
	{
		ExhaustMinionStructure* data = reinterpret_cast<ExhaustMinionStructure*>(m_buffer);
		LogWriter(m_users[data->userID]) << "Minion " << data->card->GetName() << " is exhausted.\n";
	}

	void GameInterface::BeginFirst()
	{
		BeginFirstStructure* data = reinterpret_cast<BeginFirstStructure*>(m_buffer);

		m_users[0] = data->firstUserID;
		m_users[1] = data->lastUserID;

		LogWriter(m_users[0]) << "Begin First" << std::endl;
		LogWriter(m_users[1]) << "Begin Last" << std::endl;
	}

	void GameInterface::BeginShuffle()
	{
		BeginShuffleStructure* data = reinterpret_cast<BeginShuffleStructure*>(m_buffer);
		LogWriter(m_users[data->userID]) << "Begin Shuffle" << std::endl;
	}

	void GameInterface::BeginDraw()
	{
		DrawStructure* data = reinterpret_cast<DrawStructure*>(m_buffer);

		LogWriter(m_users[data->userID]) << "Begin Draw" << std::endl;
		ShowCards(data->cards, data->numDraw);
	}

	void GameInterface::BeginMulligan()
	{
		BeginMulliganStructure* data = reinterpret_cast<BeginMulliganStructure*>(m_buffer);

		LogWriter(m_users[data->userID]) << "Begin Mulligan" << std::endl;

		int numMulligan;
		while (true)
		{
			ostream << "[*] How many cards to mulligan ? (0 ~ 3) ";
			istream >> numMulligan;

			if (numMulligan >= 0 && numMulligan <= NUM_BEGIN_DRAW)
			{
				break;
			}
		}

		BYTE mulligan[NUM_BEGIN_DRAW] = { 0, };
		for (int i = 0; i < numMulligan; ++i)
		{
			while (true)
			{
				int index = 0;
				ostream << "[*] Input card index " << i+1 << " (0 ~ 2) : ";
				istream >> index;

				if (index >= 0 && index <= NUM_BEGIN_DRAW - 1)
				{
					mulligan[i] = index;
					break;
				}
			}
		}

		// send index to agent
		m_agent.WriteBuffer(mulligan, numMulligan);
		// get new card data
		m_agent.ReadBuffer(m_buffer, sizeof(DrawStructure));
		
		LogWriter(m_users[data->userID]) << "Mulligan Result" << std::endl;

		DrawStructure* draw = reinterpret_cast<DrawStructure*>(m_buffer);
		ShowCards(draw->cards, draw->numDraw);
	}

	void GameInterface::MainReady()
	{
		MainReadyStructure* data = reinterpret_cast<MainReadyStructure*>(m_buffer);
		LogWriter(m_users[data->userID]) << "Main Ready" << std::endl;
	}

	void GameInterface::MainDraw()
	{
		DrawStructure* data = reinterpret_cast<DrawStructure*>(m_buffer);

		LogWriter(m_users[data->userID]) << "Main Draw" << std::endl;
		ShowCards(data->cards, data->numDraw);
	}

	void GameInterface::MainMenu()
	{
		MainMenuStructure* data = reinterpret_cast<MainMenuStructure*>(m_buffer);

		LogWriter(m_users[data->userID]) << "Main Menu" << std::endl;
		ShowMenus(m_mainMenuStr);

		int input;
		while (true)
		{
			ostream << "[*] Input menu : ";
			istream >> input;
			
			if (input > 0 && input <= GAME_MAIN_MENU_SIZE)
			{
				input -= 1;
				break;
			}
		}

		// pass menu index to agent
		BYTE menu = static_cast<BYTE>(input);
		m_agent.WriteBuffer(&menu, 1);
	}

	void GameInterface::MainUseCard()
	{
		MainUseCardStructure* data = reinterpret_cast<MainUseCardStructure*>(m_buffer);

		LogWriter(m_users[data->userID]) << "Main Use Card" << std::endl;
		ShowCards(data->hands, data->numHands);

		int in;
		while (true)
		{
			ostream << "Select card index (0 ~ " << static_cast<int>(data->numHands) - 1 << ") : ";

			istream >> in;
			if (in >= 0 && in < data->numHands)
			{
				if(data->hands[in]->GetCost() > data->existMana)
				{
					ostream << "Not enough mana" << std::endl;
				}
				else
				{
					break;
				}
			}
		}

		// if selected card type is minion
		if (data->hands[in]->GetCardType() == CardType::MINION)
		{
			int pos;
			while (true)
			{
				ostream << "Select Position (0 ~ " << static_cast<int>(data->numFields) << ") : ";

				istream >> pos;
				if (pos >= 0 && pos <= data->numFields)
				{
					break;
				}
			}
			
			MainUseMinionStructure minion(in, pos);
			m_agent.WriteBuffer(reinterpret_cast<BYTE*>(&minion), sizeof(MainUseMinionStructure));
		}
		else
		{
			BYTE invalid = static_cast<int>(CardType::INVALID);
			m_agent.WriteBuffer(&invalid, 1);
		}
	}

	void GameInterface::MainCombat()
	{
		MainCombatStructure* data = reinterpret_cast<MainCombatStructure*>(m_buffer);
		LogWriter(m_users[data->userID]) << "Main Combat" << std::endl;

		ostream << "User field : " << std::endl;
		ShowCards(data->currentField, data->numCurrentField);

		ostream << "Opponent field : " << std::endl;
		ShowCards(data->oppositeField, data->numOppositeField);

		int src;
		while (true)
		{
			ostream << "Select source minion (0 ~ " << static_cast<int>(data->numCurrentField) - 1 << ") : ";

			istream >> src;
			if (src >= 0 && src < data->numCurrentField)
			{
				Card** start = data->attacked;
				Card** end = data->attacked + data->numAttacked;
				if (std::find(start, end, data->currentField[src]) != end)
				{
					ostream << "Already attacked minion." << std::endl;
				}
				else
				{
					break;
				}	
			}
		}

		int dst;
		while (true)
		{
			ostream 
				<< "Select destination (0 for hero, 1 ~ " 
				<< static_cast<int>(data->numOppositeField) << " for minion) : ";

			istream >> dst;
			if (dst >= 0 && dst <= data->numOppositeField)
			{
				break;
			}
		}

		TargetingStructure targeting(src, dst);
		m_agent.WriteBuffer(reinterpret_cast<BYTE*>(&targeting), sizeof(TargetingStructure));
	}

	void GameInterface::MainEnd()
	{
		MainEndStructure* data = reinterpret_cast<MainEndStructure*>(m_buffer);
		LogWriter(m_users[data->userID]) << "Main End" << std::endl;
	}

	void GameInterface::FinalGameOver()
	{
		FinalGameOverStructure* data = reinterpret_cast<FinalGameOverStructure*>(m_buffer);
		LogWriter(m_users[data->winnerUserID]) << "Win" << std::endl;
		LogWriter(m_users[1 - data->winnerUserID]) << "Lose" << std::endl;
	}
}