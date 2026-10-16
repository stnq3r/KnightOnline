USE [KN_online]
GO

CREATE TABLE [MAGIC] (
	[MagicNum] int NOT NULL,
	[EnName] varchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS,
	[KrName] varchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS,
	[Description] varchar(100) COLLATE SQL_Latin1_General_CP1_CI_AS,
	[BeforeAction] tinyint NOT NULL,
	[TargetAction] tinyint NOT NULL,
	[SelfEffect] tinyint NOT NULL,
	[FlyingEffect] tinyint NOT NULL,
	[TargetEffect] smallint NOT NULL,
	[Moral] tinyint NOT NULL,
	[SkillLevel] smallint NOT NULL,
	[Skill] smallint NOT NULL,
	[Msp] smallint NOT NULL,
	[HP] smallint NOT NULL,
	[ItemGroup] tinyint NOT NULL,
	[UseItem] int NOT NULL,
	[CastTime] tinyint NOT NULL,
	[ReCastTime] tinyint NOT NULL,
	[SuccessRate] tinyint NOT NULL,
	[Type1] tinyint NOT NULL,
	[Type2] tinyint NOT NULL,
	[Range] smallint NOT NULL,
	[Etc] tinyint NOT NULL,
	[Event] int NOT NULL
	CONSTRAINT [PK_MAGIC] PRIMARY KEY CLUSTERED ([MagicNum])
)
GO
USE [KN_online]
GO

CREATE TABLE [MAGIC_TYPE1] (
	[iNum] int NOT NULL,
	[Name] varchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS,
	[Description] varchar(100) COLLATE SQL_Latin1_General_CP1_CI_AS,
	[Type] tinyint NOT NULL,
	[HitRate] smallint NOT NULL,
	[Hit] smallint NOT NULL,
	[AddDamage] smallint NOT NULL,
	[Delay] tinyint NOT NULL,
	[ComboType] tinyint NOT NULL,
	[ComboCount] tinyint NOT NULL,
	[ComboDamage] smallint NOT NULL,
	[Range] smallint NOT NULL
	CONSTRAINT [PK_MAGIC_TYPE1] PRIMARY KEY CLUSTERED ([iNum])
)
GO
USE [KN_online]
GO

CREATE TABLE [MAGIC_TYPE2] (
	[iNum] int NOT NULL,
	[Name] varchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS,
	[Description] varchar(100) COLLATE SQL_Latin1_General_CP1_CI_AS,
	[HitType] tinyint NOT NULL,
	[HitRate] smallint NOT NULL,
	[AddDamage] smallint NOT NULL,
	[AddRange] smallint NOT NULL,
	[NeedArrow] tinyint NOT NULL,
	[AddDamagePlus] smallint NOT NULL
	CONSTRAINT [PK_MAGIC_TYPE2] PRIMARY KEY CLUSTERED ([iNum])
)
GO
USE [KN_online]
GO

CREATE TABLE [MAGIC_TYPE3] (
	[iNum] int NOT NULL,
	[Name] varchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS,
	[Description] varchar(100) COLLATE SQL_Latin1_General_CP1_CI_AS,
	[Radius] tinyint NOT NULL,
	[Angle] smallint NOT NULL,
	[DirectType] tinyint NOT NULL,
	[FirstDamage] smallint NOT NULL,
	[EndDamage] smallint NOT NULL,
	[TimeDamage] smallint NOT NULL,
	[Duration] tinyint NOT NULL,
	[Attribute] tinyint NOT NULL
	CONSTRAINT [PK_MAGIC_TYPE3] PRIMARY KEY CLUSTERED ([iNum])
)
GO
USE [KN_online]
GO

CREATE TABLE [MAGIC_TYPE4] (
	[iNum] int NOT NULL,
	[Name] varchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS,
	[Description] varchar(100) COLLATE SQL_Latin1_General_CP1_CI_AS,
	[BuffType] tinyint NOT NULL,
	[Radius] tinyint NOT NULL,
	[Duration] smallint NOT NULL,
	[AttackSpeed] tinyint NOT NULL,
	[Speed] tinyint NOT NULL,
	[AC] smallint NOT NULL,
	[ACPct] smallint NOT NULL,
	[Attack] tinyint NOT NULL,
	[MagicAttack] tinyint NOT NULL,
	[MaxHP] smallint NOT NULL,
	[MaxHpPct] smallint NOT NULL,
	[MaxMP] smallint NOT NULL,
	[MaxMpPct] smallint NOT NULL,
	[HitRate] tinyint NOT NULL,
	[AvoidRate] smallint NOT NULL,
	[Str] smallint NOT NULL,
	[Sta] smallint NOT NULL,
	[Dex] smallint NOT NULL,
	[Intel] smallint NOT NULL,
	[Cha] smallint NOT NULL,
	[FireR] tinyint NOT NULL,
	[ColdR] tinyint NOT NULL,
	[LightningR] tinyint NOT NULL,
	[MagicR] tinyint NOT NULL,
	[DiseaseR] tinyint NOT NULL,
	[PoisonR] tinyint NOT NULL,
	[ExpPct] tinyint NOT NULL
	CONSTRAINT [PK_MAGIC_TYPE4] PRIMARY KEY CLUSTERED ([iNum])
)
GO
USE [KN_online]
GO

CREATE TABLE [MAGIC_TYPE6] (
	[iNum] int NOT NULL,
	[Name] varchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[Description] varchar(100) COLLATE SQL_Latin1_General_CP1_CI_AS,
	[Size] smallint NOT NULL,
	[TransformID] smallint NOT NULL,
	[Duration] smallint NOT NULL,
	[MaxHp] smallint NOT NULL,
	[MaxMp] smallint NOT NULL,
	[Speed] tinyint NOT NULL,
	[AttackSpeed] smallint NOT NULL,
	[TotalHit] smallint NOT NULL,
	[TotalAc] smallint NOT NULL,
	[TotalHitRate] smallint NOT NULL,
	[TotalEvasionRate] smallint NOT NULL,
	[TotalFireR] smallint NOT NULL,
	[TotalColdR] smallint NOT NULL,
	[TotalLightningR] smallint NOT NULL,
	[TotalMagicR] smallint NOT NULL,
	[TotalDiseaseR] smallint NOT NULL,
	[TotalPoisonR] smallint NOT NULL,
	[Class] smallint NOT NULL,
	[UserSkillUse] tinyint NOT NULL,
	[NeedItem] tinyint NOT NULL,
	[SkillSuccessRate] tinyint NOT NULL,
	[MonsterFriendly] tinyint NOT NULL
	CONSTRAINT [PK_MAGIC_TYPE6] PRIMARY KEY CLUSTERED ([iNum])
)
GO
USE [KN_online]
GO

CREATE TABLE [MAGIC_TYPE7] (
	[nIndex] int NOT NULL,
	[strName] varchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS,
	[strNote] varchar(100) COLLATE SQL_Latin1_General_CP1_CI_AS,
	[byValidGroup] tinyint NOT NULL,
	[byNatoinChange] tinyint NOT NULL,
	[shMonsterNum] smallint NOT NULL,
	[byTargetChange] tinyint NOT NULL,
	[byStateChange] tinyint NOT NULL,
	[byRadius] tinyint NOT NULL,
	[shHitrate] smallint NOT NULL,
	[shDuration] smallint NOT NULL,
	[shDamage] smallint NOT NULL,
	[byVisoin] tinyint NOT NULL,
	[nNeedItem] int NOT NULL
	CONSTRAINT [PK_MAGIC_TYPE7] PRIMARY KEY CLUSTERED ([nIndex])
)
GO
USE [KN_online]
GO

CREATE TABLE [MAGIC_TYPE9] (
	[iNum] int NOT NULL,
	[Name] varchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS,
	[Description] varchar(100) COLLATE SQL_Latin1_General_CP1_CI_AS,
	[ValidGroup] tinyint NOT NULL,
	[NationChange] tinyint NOT NULL,
	[MonsterNum] smallint NOT NULL,
	[TargetChange] tinyint NOT NULL,
	[StateChange] tinyint NOT NULL,
	[Radius] smallint NOT NULL,
	[Hitrate] smallint NOT NULL,
	[Duration] smallint NOT NULL,
	[AddDamage] smallint NOT NULL,
	[Vision] smallint NOT NULL,
	[NeedItem] int NOT NULL
	CONSTRAINT [PK_MAGIC_TYPE9] PRIMARY KEY CLUSTERED ([iNum])
)
GO
