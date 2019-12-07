#include <windows.h>
#include <stdio.h>
#include "discord-rpc/include/discord_rpc.h"
#pragma comment(lib, "discord-rpc/lib/discord-rpc.lib")

const char* APPLICATION_ID = "652718840035409920";
DiscordRichPresence discordPresence;
bool initialized = false;

void handleDiscordReady(const DiscordUser *request) {}

void handleDiscordDisconnected(int errcode, const char* message) {}

void handleDiscordError(int errcode, const char* message) {}

void handleDiscordJoin(const char* secret) {}

void handleDiscordSpectate(const char* secret) {}

void handleDiscordJoinRequest(const DiscordUser *request) {}

void InitDiscord()
{
	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));
	handlers.ready = handleDiscordReady;
	handlers.disconnected = handleDiscordDisconnected;
	handlers.errored = handleDiscordError;
	handlers.joinGame = handleDiscordJoin;
	handlers.spectateGame = handleDiscordSpectate;
	handlers.joinRequest = handleDiscordJoinRequest;
	Discord_Initialize(APPLICATION_ID, &handlers, 1, NULL);
}

void InitPresence()
{
	if (initialized) return;
	memset(&discordPresence, 0, sizeof(discordPresence));
	initialized = true;
}

char oldState[256];

void UpdatePresence(char *songname)
{
	InitPresence();

	char songinfo[256];

	sprintf_s(songinfo, 256, songname);

	discordPresence.largeImageKey = "icon";
	discordPresence.state = songinfo;
	sprintf_s(oldState, 256, songinfo);
	discordPresence.instance = 1;
	Discord_UpdatePresence(&discordPresence);
}