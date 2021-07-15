
#include "jump_ghost.h"
#include "jump_utils.h"
#include "jump_scores.h"
#include <cassert>
#include <fstream>
#include <filesystem>

namespace Jump
{

/// <summary>
/// Define the private variables
/// </summary>
std::vector<ghost_data_t*> GhostReplay::_ghosts;
std::vector<std::string> GhostReplay::_ghostModels;
bool GhostReplay::_paused = false;

/// <summary>
/// Creates a new entity for the ghost.  Does not load the replay data.
/// </summary>
void GhostReplay::Init()
{
	_ghosts.assign(200, nullptr);

	CreateGhostEntity();
}

edict_t* GhostReplay::CreateGhostEntity()
{
	auto ghost = G_Spawn();
	ghost->svflags = SVF_NOCLIENT;	// hides the ent
	ghost->movetype = MOVETYPE_NOCLIP;
	ghost->clipmask = MASK_SOLID;
	ghost->solid = SOLID_NOT;
	VectorClear(ghost->mins);
	VectorClear(ghost->maxs);
	VectorClear(ghost->s.angles);
	VectorClear(ghost->s.old_origin);
	VectorClear(ghost->s.origin);
	ghost->dmg = 0;
	ghost->classname = "ghost";

	std::string model = PickRandomGhostModel();
	std::string modelPath = std::string("players/ghost/") + model + ".md2";

	ghost->s.modelindex = gi.modelindex(const_cast<char*>(modelPath.c_str()));
	ghost->s.modelindex2 = 0;
	ghost->s.modelindex3 = 0;
	ghost->s.modelindex4 = 0;
	ghost->s.skinnum = ghost - g_edicts - 1;
	ghost->s.frame = 0;
	gi.unlinkentity(ghost);


	auto skin = va("abcdef\\female/ctf_b");

	gi.WriteByte(svc_configstring);
	gi.WriteShort(CS_PLAYERSKINS + ghost - g_edicts - 1);
	gi.WriteString(const_cast<char*>(skin));
	gi.unicast(ent, true);

	return ghost;
}

/// <summary>
/// Loads the top time replay data for the ghost.
/// </summary>
void GhostReplay::LoadReplay()
{
	for (int i = 0; i < _ghosts.size(); i++ )
	{
		auto ghost = _ghosts[i];

		if (!ghost)
		{
			ghost = new ghost_data_t();
			_ghosts[i] = ghost;
		}

		int timeMs = 0;
		std::string username;
		LocalDatabase::GetReplayByPosition(level.mapname, i + 1, ghost->replay, timeMs, username);

		if (ghost->replay.size() > 0)
		{
			ghost->ghost = CreateGhostEntity();
		}
	}
}

/// <summary>
/// Advances the ghost position by one frame.
/// </summary>
void GhostReplay::RunFrame()
{
	for(auto ghost : _ghosts) {
		if (!ghost) continue;

		if (!ghost->ghost) continue;

		if (ghost->replay.empty())
		{
			gi.unlinkentity(ghost->ghost);
			return;
		}


		auto ghost_ent = ghost->ghost;

		if (ghost->frame >= 0 && ghost->frame < ghost->replay.size())
		{
			auto& replay_frame = ghost->replay[ghost->frame];

			VectorCopy(replay_frame.pos, ghost_ent->s.origin);
			VectorCopy(replay_frame.pos, ghost_ent->s.old_origin);
			VectorCopy(replay_frame.angles, ghost_ent->s.angles);
			ghost_ent->s.frame = replay_frame.animation_frame;
			ghost_ent->svflags = SVF_PROJECTILE;
			gi.linkentity(ghost_ent);

			if (!_paused)
				ghost->frame++;
		}
	}
}

void GhostReplay::TogglePlay()
{
	_paused = !_paused;
}

void GhostReplay::Restart()
{
	for(auto ghost : _ghosts) {
		if (!ghost) continue;

		ghost->frame = 0;
	}
}

/// <summary>
/// Load all the ghost models from the jump/27910/ghost_models.cfg file.
/// </summary>
void GhostReplay::LoadGhostModels()
{
	_ghostModels.clear();
	std::string filename = GetModPortDir() + "/ghost_models.cfg";
	std::ifstream file(filename);
	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			std::string modelPath = GetModDir() + "/players/ghost/" + line + ".md2";
			if (std::filesystem::exists(modelPath))
			{
				_ghostModels.push_back(line);
			}
		}
	}
}

/// <summary>
/// Chooses a random ghost model from the full loaded list.
/// </summary>
/// <returns></returns>
std::string GhostReplay::PickRandomGhostModel()
{
	std::string model = "penguin"; // default if nothing is loaded
	if (_ghostModels.size() > 0)
	{
		int index = ::rand() % _ghostModels.size();
		model = _ghostModels[index];
	}
	return model;
}

}