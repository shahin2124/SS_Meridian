#include "Mission3Config.h"
namespace m3 {
Config::Config() : playerMaxHealth(100), kickDamage(10), weaponDamage(22), fireDamage(14),
 playerSpeed(245), monsterStaggerCooldown(2.8f), projectileSpeed(630), fightDuration(60), invulnerability(.8f),guardMultiplier(.25f),
 kickDuration(.60f),weaponDuration(.84f),fireDuration(.78f),hitDuration(.66f),guardDuration(.66f),
 kickCooldown(.8f),weaponCooldown(1.15f),fireCooldown(1.25f),guardCooldown(1.0f),
 telegraphDuration(.72f),attackDuration(.90f),recoveryDuration(1.05f),defeatDuration(1.20f),
 kickRange(165),weaponRange(265),jumpDuration(.92f),jumpHeight(105),
 introDuration(.55f),walkDistance(2450),completeDuration(2.8f),claimedDuration(.9f),showTimer(true) {
    monsterHealth[0]=420; monsterHealth[1]=480; monsterHealth[2]=540;
    monsterSpeed[0]=95; monsterSpeed[1]=120; monsterSpeed[2]=85;
    monsterDamage[0][0]=19;monsterDamage[0][1]=12;
    monsterDamage[1][0]=17;monsterDamage[1][1]=14;
    monsterDamage[2][0]=24;monsterDamage[2][1]=20;
    monsterRange[0][0]=240;monsterRange[0][1]=175;
    monsterRange[1][0]=310;monsterRange[1][1]=185;
    monsterRange[2][0]=245;monsterRange[2][1]=350;
}
const GuideData Guides[3]={
 {"HEAVY ARMORED MELEE","Club swing; close shield strike.","Step back from the swing. Guard the shield.","Use Mind fire at range; Space weapon in recovery."},
 {"BLADE + CORRUPTED CLAW","Long blade swings; close claw strikes.","Leave the blade arc; back away from the claw.","Use Mind fire; punish missed attacks with Space weapon."},
 {"ARM SMASH + SPINAL ATTACK","Crushing arms and an extended spinal lash.","Back away from the smash; guard or jump the lash.","Make distance for Mind fire; use Space weapon in recovery."}
};
}
