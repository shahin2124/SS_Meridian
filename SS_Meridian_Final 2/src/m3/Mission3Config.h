#pragma once
namespace m3 {
// TUNABLE / NON-CANON GAMEPLAY BALANCE. Artwork-derived patterns are prototypes,
// not permanent lore, weapon identities, or final approved monster powers.
struct Config {
    float playerMaxHealth, monsterHealth[3], monsterDamage[3][2];
    float kickDamage, weaponDamage, fireDamage, playerSpeed, monsterSpeed[3];
    float monsterStaggerCooldown;
    float projectileSpeed, fightDuration, invulnerability, guardMultiplier;
    float kickDuration, weaponDuration, fireDuration, hitDuration, guardDuration;
    float kickCooldown, weaponCooldown, fireCooldown, guardCooldown;
    float telegraphDuration, attackDuration, recoveryDuration, defeatDuration;
    float monsterRange[3][2], kickRange, weaponRange, jumpDuration, jumpHeight;
    float introDuration, walkDistance, completeDuration, claimedDuration;
    bool showTimer;
    // Issac receives 500 HP only against Monster 3, including retries.
    float playerHealthForFight(int monsterIndex) const { return monsterIndex==2 ? 500.0f : playerMaxHealth; }
    Config();
};
struct Keys {
    enum { Left='A', Right='D', Jump='W', Kick='J', Weapon='K', Fire='L',
           Guard=16, Confirm=13, Exit=27 };
};
struct Layout {
    enum { Width=1280, Height=720, Ground=646, PlayerStart=280, MonsterStart=960,
           PlayerMin=205, PlayerMax=765, MonsterMin=440, MonsterMax=975,
           Separation=140, IntroStart=180, TriggerX=410 };
    static float cageX(int n) { const float x[3]={665,848,1060}; return x[n]; }
};
struct GuideData { const char *power,*attack,*dodge,*defeat; };
extern const GuideData Guides[3];
}
