#pragma once

#include "g_local.h"

namespace Jump
{
struct ghost_data_t
{
	std::vector<replay_frame_t> replay;
	
    size_t frame = 0;
	
    edict_t* ghost = nullptr;
};


class GhostReplay
{
public:
    static void Init();
    static void LoadReplay();
    static void RunFrame();
    static void LoadGhostModels();
    static void TogglePlay();
    static void Restart();
private:
    static edict_t* CreateGhostEntity();

    static std::string PickRandomGhostModel();

    static std::vector<ghost_data_t*> _ghosts;
    static std::vector<std::string> _ghostModels;

    static bool _paused;
};

}