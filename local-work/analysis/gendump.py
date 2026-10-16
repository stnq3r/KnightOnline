cols = {
 "MAGIC": "MagicNum EnName KrName Description BeforeAction TargetAction SelfEffect FlyingEffect TargetEffect Moral SkillLevel Skill Msp HP ItemGroup UseItem CastTime ReCastTime SuccessRate Type1 Type2 Range Etc Event",
 "MAGIC_TYPE1": "iNum Name Description Type HitRate Hit AddDamage Delay ComboType ComboCount ComboDamage Range",
 "MAGIC_TYPE2": "iNum Name Description HitType HitRate AddDamage AddRange NeedArrow AddDamagePlus",
 "MAGIC_TYPE3": "iNum Name Description Radius Angle DirectType FirstDamage EndDamage TimeDamage Duration Attribute",
 "MAGIC_TYPE4": "iNum Name Description BuffType Radius Duration AttackSpeed Speed AC ACPct Attack MagicAttack MaxHP MaxHpPct MaxMP MaxMpPct HitRate AvoidRate Str Sta Dex Intel Cha FireR ColdR LightningR MagicR DiseaseR PoisonR ExpPct",
 "MAGIC_TYPE5": "iNum Name Description Type ExpRecover NeedStone",
 "MAGIC_TYPE6": "iNum Name Description Size TransformID Duration MaxHp MaxMp Speed AttackSpeed TotalHit TotalAc TotalHitRate TotalEvasionRate TotalFireR TotalColdR TotalLightningR TotalMagicR TotalDiseaseR TotalPoisonR Class UserSkillUse NeedItem SkillSuccessRate MonsterFriendly",
 "MAGIC_TYPE7": "nIndex strName strNote byValidGroup byNatoinChange shMonsterNum byTargetChange byStateChange byRadius shHitrate shDuration shDamage byVisoin nNeedItem",
 "MAGIC_TYPE8": "iNum Name Description Target Radius WarpType ExpRecover",
 "MAGIC_TYPE9": "iNum Name Description ValidGroup NationChange MonsterNum TargetChange StateChange Radius Hitrate Duration AddDamage Vision NeedItem",
}
out = ["SET NOCOUNT ON;"]
for t, c in cols.items():
    cs = c.split()
    fields = ["ISNULL(CAST(%s AS varchar(max)),'')" % x for x in cs]
    expr = "'" + t + "|'+" + "+'|'+".join(fields)
    out.append("SELECT %s FROM %s;" % (expr, t))
out.append("GO")
open("/tmp/dumpmagic.sql", "w").write("\n".join(out) + "\n")
print(out[1][:160])
