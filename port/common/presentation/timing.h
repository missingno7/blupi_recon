#pragma once
#include <cstdint>
#include <cstdio>
#include "platform/host.h"

// Historical single-pending-tick schedule: drop missed ticks instead of
// catching up. Presentation can run between these fixed simulation ticks.
inline uint64_t PortNextTick(uint64_t next, uint64_t now, uint64_t interval)
{
	next += interval;
	return next <= now ? now + interval : next;
}

inline double PortFrameAlpha(uint64_t now, uint64_t lastTick, uint64_t interval)
{
	const double alpha = (double)(now - lastTick) / (double)interval;
	return alpha < 1.0 ? alpha : 1.0;
}

// Presentation timing, logged every 5 s with --verbose: presents per second,
// the cost of drawing a presented frame (render: scene to back buffer or wide
// target) and of handing it to the display (present: conversion, texture
// upload, SDL_RenderPresent including the wait for vsync), and of a game tick.
struct PortPerf
{
	uint64_t since = 0;
	long     presents = 0, ticks = 0, firstFrame = 0;
	uint64_t render = 0, renderMax = 0, present = 0, presentMax = 0, tick = 0, tickMax = 0;

	void Tick(uint64_t ns)
	{
		ticks++;
		tick += ns;
		if (ns > tickMax) tickMax = ns;
	}

	void Present(uint64_t renderNs, uint64_t presentNs, uint64_t now, long frame, int width)
	{
		if (since == 0) { since = now; firstFrame = frame; }
		presents++;
		render += renderNs;
		present += presentNs;
		if (renderNs > renderMax) renderMax = renderNs;
		if (presentNs > presentMax) presentMax = presentNs;
		if (now - since < 5000000000ull) return;
		const double seconds = (now - since) / 1e9;
		char text[240];
		snprintf(text, sizeof(text),
				 "perf: %.0f presents/s (render avg %.2f max %.2f ms, present avg %.2f max %.2f ms), "
				 "%.1f game frames/s (tick avg %.2f max %.2f ms), view %d",
				 presents / seconds, render / 1e6 / presents, renderMax / 1e6, present / 1e6 / presents,
				 presentMax / 1e6, (frame - firstFrame) / seconds, ticks ? tick / 1e6 / ticks : 0.0,
				 tickMax / 1e6, width);
		blupi::host::log_debug(text);
		*this = PortPerf();
	}
};

