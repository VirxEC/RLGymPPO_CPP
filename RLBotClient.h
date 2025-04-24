#pragma once

#include <rlbot/Bot.h>
#include <RLGymSim_CPP/Utils/OBSBuilders/OBSBuilder.h>
#include <RLGymSim_CPP/Utils/ActionParsers/ActionParser.h>

#include <RLGymPPO_CPP/Util/InferUnit.h>

struct RLBotParams {
	RLGSC::OBSBuilder* obsBuilder = NULL; // Use your OBS builder
	RLGSC::ActionParser* actionParser = NULL; // Use your action parser

	std::filesystem::path policyPath; // The path to your trained PPO_POLICY.lt
	int obsSize; // You can find this from the console when running training
	std::vector<int> policyLayerSizes = {}; // Your layer sizes
	int tickSkip; // Your tick skip
};

class RLBotBot : public rlbot::Bot {
public:

	// Parameters to define the bot
	RLBotParams params;

	// Inference unit to infer the policy with, also uses our obs and action parser
	RLGPC::InferUnit* policyInferUnit;

	// Queued action and current action
	RLGSC::Action 
		action = {}, 
		controls = {};

	// Persistent info
	bool updateAction = true;
	float prevTime = 0;
	int ticks = -1;

	RLBotBot () noexcept = delete;

	~RLBotBot () noexcept override;

	RLBotBot (std::unordered_set<unsigned> indices_, unsigned team_, std::string name_) noexcept;

	RLBotBot (RLBotBot const &) noexcept = delete;

	RLBotBot (RLBotBot &&) noexcept = delete;

	RLBotBot &operator= (RLBotBot const &) noexcept = delete;

	RLBotBot &operator= (RLBotBot &&) noexcept = delete;

	void update (rlbot::flat::GamePacket const *packet_,
	    rlbot::flat::BallPrediction const *ballPrediction_) noexcept override;
};
