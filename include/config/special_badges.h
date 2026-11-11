#ifndef GUARD_CONFIG_SPECIAL_BADGES_H
#define GUARD_CONFIG_SPECIAL_BADGES_H

// Exact natural rates, one success in DENOMINATOR encounters.
// Alpha, Enraged and Omega are confirmed; others are balancing proposals.
#define SPECIAL_BADGE_ALPHA_DENOMINATOR      100
#define SPECIAL_BADGE_SUPER_DENOMINATOR      180
#define SPECIAL_BADGE_ENRAGED_DENOMINATOR     90
#define SPECIAL_BADGE_OMEGA_DENOMINATOR     9000
#define SPECIAL_BADGE_HYPER_DENOMINATOR      250
#define SPECIAL_BADGE_OVERLOADED_DENOMINATOR 350
#define SPECIAL_BADGE_TITAN_DENOMINATOR      500
#define SPECIAL_BADGE_ANCESTRAL_DENOMINATOR  750
#define SPECIAL_BADGE_ASCENDED_DENOMINATOR  2500

#define SPECIAL_BADGE_SECOND_GATE 12
#define SPECIAL_BADGE_THIRD_GATE  40
#define SPECIAL_BADGE_FOURTH_GATE 100

// Player-only special acquisition (starter, daycare, scripted gift, NPC trade):
// +15% RELATIVE success chance for tiers C/B.  Never +15 percentage points.
// Tier A, S, and Omega retain their ordinary rates.
#define SPECIAL_BADGE_SPECIAL_ORIGIN_LOW_TIER_BOOST_PERCENT 15

// A paid Mutator Omega roll (10/65536) is proposed but not active.
#define SPECIAL_BADGE_MUTATOR_OMEGA_ROLLS 10

// Capture odds are intentionally harsh for special individuals.
#define SPECIAL_BADGE_ALPHA_CAPTURE_PERCENT 75
#define SPECIAL_BADGE_SUPER_CAPTURE_PERCENT 35
#define SPECIAL_BADGE_ENRAGED_CAPTURE_PERCENT 12
#define SPECIAL_BADGE_OMEGA_CAPTURE_PERCENT 3

#endif // GUARD_CONFIG_SPECIAL_BADGES_H
