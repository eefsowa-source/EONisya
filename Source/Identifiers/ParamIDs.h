#pragma once

namespace ParamIDs
{
    // ── Mode / navigation ────────────────────────────────────────────────
    inline constexpr auto page            = "page";            // 0 perform / 1 voice / 2 mixing

    // ── Assignable knobs (perform page, set 2/4) ─────────────────────────
    inline constexpr auto cutoff          = "cutoff";
    inline constexpr auto reso            = "reso";
    inline constexpr auto attack          = "attack";
    inline constexpr auto release         = "release";
    inline constexpr auto assign1         = "assign1";
    inline constexpr auto assign2         = "assign2";
    inline constexpr auto reverb          = "reverb";
    inline constexpr auto chorus          = "chorus";

    // ── Voice edit knobs ─────────────────────────────────────────────────
    inline constexpr auto drive           = "drive";
    inline constexpr auto keyFollow       = "keyFollow";
    inline constexpr auto fegAtk          = "fegAtk";
    inline constexpr auto fegDcy          = "fegDcy";
    inline constexpr auto fegSus          = "fegSus";
    inline constexpr auto fegRel          = "fegRel";

    // ── Mixing knobs ─────────────────────────────────────────────────────
    inline constexpr auto volume          = "volume";
    inline constexpr auto pan             = "pan";
    inline constexpr auto revSend         = "revSend";
    inline constexpr auto choSend         = "choSend";

    // ── Transport / status (non-automatable UI state) ────────────────────
    inline constexpr auto tempo            = "tempo";   // 40.0 – 300.0 BPM
    inline constexpr auto masterLevel      = "master";  // 0 – 127
}
