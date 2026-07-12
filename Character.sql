USE [MuOnline]
GO

/****** Object:  Table [dbo].[Character]    Script Date: 8/7/2026 23:55:05 ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

CREATE TABLE [dbo].[Character](
	[AccountID] [varchar](10) NOT NULL,
	[Name] [varchar](10) NOT NULL,
	[cLevel] [int] NULL,
	[LevelUpPoint] [int] NULL,
	[Class] [tinyint] NULL,
	[Experience] [int] NULL,
	[Strength] [int] NULL,
	[Dexterity] [int] NULL,
	[Vitality] [int] NULL,
	[Energy] [int] NULL,
	[Leadership] [int] NULL,
	[Inventory] [varbinary](3776) NULL,
	[MagicList] [varbinary](180) NULL,
	[Money] [int] NULL,
	[Life] [real] NULL,
	[MaxLife] [real] NULL,
	[Mana] [real] NULL,
	[MaxMana] [real] NULL,
	[BP] [real] NULL,
	[MaxBP] [real] NULL,
	[Shield] [real] NULL,
	[MaxShield] [real] NULL,
	[MapNumber] [smallint] NULL,
	[MapPosX] [smallint] NULL,
	[MapPosY] [smallint] NULL,
	[MapDir] [tinyint] NULL,
	[PkCount] [int] NULL,
	[PkLevel] [int] NULL,
	[PkTime] [int] NULL,
	[MDate] [smalldatetime] NULL,
	[LDate] [smalldatetime] NULL,
	[CtlCode] [tinyint] NULL,
	[DbVersion] [tinyint] NULL,
	[Quest] [varbinary](50) NULL,
	[ChatLimitTime] [smallint] NULL,
	[FruitPoint] [int] NULL,
	[EffectList] [varbinary](208) NULL,
	[FruitAddPoint] [int] NOT NULL,
	[FruitSubPoint] [int] NOT NULL,
	[ResetCount] [int] NOT NULL,
	[MasterResetCount] [int] NOT NULL,
	[ExtInventory] [int] NOT NULL,
	[Kills] [int] NOT NULL,
	[Deads] [int] NOT NULL,
	[Bloc_Expire] [smalldatetime] NULL,
	[TheGift] [int] NOT NULL,
	[ResetTime] [smalldatetime] NULL,
	[ResetVIP] [int] NOT NULL,
	[Resets_Time] [int] NOT NULL,
	[rDanhHieu] [int] NOT NULL,
	[rTuLuyen] [int] NOT NULL,
	[rThueVip] [int] NOT NULL,
	[rThueVipTime] [int] NOT NULL,
	[rResetLife] [int] NOT NULL,
	[CTCTime] [int] NOT NULL,
	[CTCRegDay] [int] NOT NULL,
 CONSTRAINT [PK_Character] PRIMARY KEY NONCLUSTERED 
(
	[Name] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF_Character_cLevel]  DEFAULT ((1)) FOR [cLevel]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF_Character_LevelUpPoint]  DEFAULT ((0)) FOR [LevelUpPoint]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF_Character_Experience]  DEFAULT ((0)) FOR [Experience]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF__Character__Leade__7A672E12]  DEFAULT ((0)) FOR [Leadership]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF_Character_Money]  DEFAULT ((0)) FOR [Money]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF_Character_MapDir]  DEFAULT ((0)) FOR [MapDir]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF_Character_PkCount]  DEFAULT ((0)) FOR [PkCount]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF_Character_PkLevel]  DEFAULT ((3)) FOR [PkLevel]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF_Character_PkTime]  DEFAULT ((0)) FOR [PkTime]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF_Character_CtlCode]  DEFAULT ((0)) FOR [CtlCode]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF__Character__DbVer__787EE5A0]  DEFAULT ((0)) FOR [DbVersion]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF__Character__Quest__797309D9]  DEFAULT ((0)) FOR [Quest]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF__Character__ChatL__7B5B524B]  DEFAULT ((0)) FOR [ChatLimitTime]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF_Character_FruitPoint]  DEFAULT ((0)) FOR [FruitPoint]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF__Character__Fruit__4F67C174]  DEFAULT ((0)) FOR [FruitAddPoint]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF__Character__Fruit__505BE5AD]  DEFAULT ((0)) FOR [FruitSubPoint]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF__Character__Reset__515009E6]  DEFAULT ((0)) FOR [ResetCount]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF__Character__Maste__7CF981FA]  DEFAULT ((0)) FOR [MasterResetCount]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF__Character__ExtIn__40E497F3]  DEFAULT ((0)) FOR [ExtInventory]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF_Character_Kills]  DEFAULT ((0)) FOR [Kills]
GO

ALTER TABLE [dbo].[Character] ADD  CONSTRAINT [DF_Character_Deads]  DEFAULT ((0)) FOR [Deads]
GO

ALTER TABLE [dbo].[Character] ADD  DEFAULT ((0)) FOR [TheGift]
GO

ALTER TABLE [dbo].[Character] ADD  DEFAULT ((0)) FOR [ResetVIP]
GO

ALTER TABLE [dbo].[Character] ADD  DEFAULT ((0)) FOR [Resets_Time]
GO

ALTER TABLE [dbo].[Character] ADD  DEFAULT ((0)) FOR [rDanhHieu]
GO

ALTER TABLE [dbo].[Character] ADD  DEFAULT ((0)) FOR [rTuLuyen]
GO

ALTER TABLE [dbo].[Character] ADD  DEFAULT ((0)) FOR [rThueVip]
GO

ALTER TABLE [dbo].[Character] ADD  DEFAULT ((0)) FOR [rThueVipTime]
GO

ALTER TABLE [dbo].[Character] ADD  DEFAULT ((0)) FOR [rResetLife]
GO

ALTER TABLE [dbo].[Character] ADD  DEFAULT ((0)) FOR [CTCTime]
GO

ALTER TABLE [dbo].[Character] ADD  DEFAULT ((0)) FOR [CTCRegDay]
GO

/* PARCHE */
USE [MuOnline]
GO

IF COL_LENGTH('dbo.Character', 'UserSkinPick') IS NULL
BEGIN
    ALTER TABLE [dbo].[Character]
    ADD [UserSkinPick] [int] NOT NULL
        CONSTRAINT [DF_Character_UserSkinPick] DEFAULT ((0))
END
GO

IF COL_LENGTH('dbo.Character', 'PheHanhTau') IS NULL
BEGIN
    ALTER TABLE [dbo].[Character]
    ADD [PheHanhTau] [tinyint] NOT NULL
        CONSTRAINT [DF_Character_PheHanhTau] DEFAULT ((0))
END
GO

IF COL_LENGTH('dbo.Character', 'HonHoan') IS NULL
BEGIN
    ALTER TABLE [dbo].[Character]
    ADD [HonHoan] [tinyint] NOT NULL
        CONSTRAINT [DF_Character_HonHoan] DEFAULT ((0))
END
GO

IF COL_LENGTH('dbo.Character', 'PointUsePhe') IS NULL
BEGIN
    ALTER TABLE [dbo].[Character]
    ADD [PointUsePhe] [int] NOT NULL
        CONSTRAINT [DF_Character_PointUsePhe] DEFAULT ((0))
END
GO

IF COL_LENGTH('dbo.Character', 'LuotDiEndLess') IS NULL
BEGIN
    ALTER TABLE [dbo].[Character]
    ADD [LuotDiEndLess] [smallint] NOT NULL
        CONSTRAINT [DF_Character_LuotDiEndLess] DEFAULT ((0))
END
GO

IF COL_LENGTH('dbo.Character', 'MOCNAPCOIN') IS NULL
BEGIN
    ALTER TABLE [dbo].[Character]
    ADD [MOCNAPCOIN] [int] NOT NULL
        CONSTRAINT [DF_Character_MOCNAPCOIN] DEFAULT ((0))
END
GO