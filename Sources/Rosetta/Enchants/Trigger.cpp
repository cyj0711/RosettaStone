// This code is based on Sabberstone project.
// Copyright (c) 2017-2019 SabberStone Team, darkfriend77 & rnilva
// RosettaStone is hearthstone simulator using C++ with reinforcement learning.
// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeungHyun Jeon

#include <Rosetta/Enchants/Trigger.hpp>

namespace RosettaStone
{
Trigger::Trigger(TriggerType type) : m_triggerType(type)
{
    // Do nothing
}

void Trigger::Activate(Entity& owner)
{
    (void)owner;
}
}  // namespace RosettaStone