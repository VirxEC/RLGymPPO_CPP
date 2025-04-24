#include "RLBotClient.h"

using namespace RLGSC;
using namespace RLGPC;

RLBotBot::RLBotBot (std::unordered_set<unsigned> indices_, unsigned const team_, std::string name_) noexcept
    : rlbot::Bot (std::move (indices_), team_, std::move (name_))
{
	std::set<unsigned> sorted (std::begin (indices), std::end (indices));
	for (auto const &index : sorted)
		std::printf ("Team %u Index %u: %s created\n", team_, index, name_.c_str());
}

RLBotBot::~RLBotBot() {
	delete policyInferUnit;
}

Vec ToVec(const rlbot::flat::Vector3 rlbotVec) {
	return Vec(rlbotVec.x(), rlbotVec.y(), rlbotVec.z());
}

PhysObj ToPhysObj(const rlbot::flat::Physics* phys) {
	PhysObj obj = {};
	obj.pos = ToVec(phys->location());

	Angle ang = Angle(phys->rotation().yaw(), phys->rotation().pitch(), phys->rotation().roll());
	obj.rotMat = ang.ToRotMat();

	obj.vel = ToVec(phys->velocity());
	obj.angVel = ToVec(phys->angular_velocity());

	return obj;
}

PlayerData ToPlayer(const rlbot::flat::PlayerInfo* playerInfo) {
	PlayerData pd = {};
	pd.carId = playerInfo->spawn_id();

	pd.team = (Team)playerInfo->team();

	pd.phys = ToPhysObj(playerInfo->physics());
	pd.physInv = pd.phys.Invert();

	pd.carState.pos = pd.phys.pos;
	pd.carState.rotMat = pd.phys.rotMat;
	pd.carState.vel = pd.phys.vel;
	pd.carState.angVel = pd.phys.angVel;

	pd.boostFraction = playerInfo->boost() / 100.f;
	pd.carState.isOnGround = playerInfo->air_state() == rlbot::flat::AirState::OnGround;
	pd.carState.hasJumped = playerInfo->has_jumped();
	pd.carState.hasDoubleJumped = playerInfo->has_double_jumped();
	pd.carState.isDemoed = playerInfo->demolished_timeout() < 0;
	pd.hasFlip = !playerInfo->has_dodged() && !playerInfo->has_double_jumped();

	return pd;
}

GameState ToGameState(rlbot::flat::GamePacket const *packet) {
	GameState gs = {};

	auto players = packet->players();
	for (int i = 0; i < players->size(); i++)
		gs.players.push_back(ToPlayer(players->Get(i)));

	gs.ball = ToPhysObj(packet->balls()->Get(0)->physics());
	gs.ballInv = gs.ball.Invert();

	auto boostPadStates = packet->boost_pads();
	if (boostPadStates->size() != CommonValues::BOOST_LOCATIONS_AMOUNT) {
		if (rand() % 20 == 0) { // Don't spam-log as that will lag the bot
			RG_LOG(
				"RLBotClient ToGameState(): Bad boost pad amount, expected " << CommonValues::BOOST_LOCATIONS_AMOUNT << " but got " << boostPadStates->size()
			);
		}

		// Just set all boost pads to on
		std::fill(gs.boostPads.begin(), gs.boostPads.end(), 1);
	} else {
		for (int i = 0; i < CommonValues::BOOST_LOCATIONS_AMOUNT; i++) {
			gs.boostPads[i] = boostPadStates->Get(i)->is_active();
			gs.boostPadsInv[CommonValues::BOOST_LOCATIONS_AMOUNT - i - 1] = gs.boostPads[i];
		}
	}

	return gs;
}

void RLBotBot::update (rlbot::flat::GamePacket const *packet,
	rlbot::flat::BallPrediction const *ballPrediction_) noexcept {
	for (auto const &index : this->indices)
	{
		// If there's no ball, there's nothing to chase; don't do anything
		if (packet->balls ()->size () == 0)
		{
			setOutput (index, {});
			continue;
		}

		// We're not in the game packet; skip this tick
		if (packet->players ()->size () <= index)
		{
			setOutput (index, {});
			continue;
		}

		float curTime = packet->match_info()->seconds_elapsed();
		float deltaTime = curTime - prevTime;
		prevTime = curTime;

		int ticksElapsed = roundf(deltaTime * 120);
		ticks += ticksElapsed;

		GameState gs = ToGameState(packet);
		auto& localPlayer = gs.players[index];

		if (updateAction) {
			updateAction = false;
			action = policyInferUnit->InferPolicySingle(localPlayer, gs, controls, true);
		}

		if (ticks >= params.tickSkip || ticks == -1) {
			// Apply new action
			controls = action;

			// Trigger action update next tick
			ticks = 0;
			updateAction = true;
		}

		setOutput(index, {
			controls.throttle,
			controls.steer,
			controls.pitch,
			controls.yaw,
			controls.roll,
			controls.jump,
			controls.boost,
			controls.handbrake,
			false, // use_item
		});
	}
}
