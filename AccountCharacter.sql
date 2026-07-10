USE [MuOnline]
GO

SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

CREATE TABLE [dbo].[AccountCharacter]
(
	[Number]         INT IDENTITY(1,1) NOT NULL,
	[Id]             VARCHAR(10) NOT NULL,

	[GameID1]        VARCHAR(10) NULL,
	[GameID2]        VARCHAR(10) NULL,
	[GameID3]        VARCHAR(10) NULL,
	[GameID4]        VARCHAR(10) NULL,
	[GameID5]        VARCHAR(10) NULL,
	[GameIDC]        VARCHAR(10) NULL,

	[MoveCnt]        TINYINT NOT NULL
		CONSTRAINT [DF_AccountCharacter_MoveCnt] DEFAULT (0),

	[ExtClass]       INT NOT NULL
		CONSTRAINT [DF_AccountCharacter_ExtClass] DEFAULT (0),

	[ExtWarehouse]   INT NOT NULL
		CONSTRAINT [DF_AccountCharacter_ExtWarehouse] DEFAULT (0),

	CONSTRAINT [PK_AccountCharacter]
		PRIMARY KEY CLUSTERED ([Number] ASC),

	CONSTRAINT [UQ_AccountCharacter_Id]
		UNIQUE NONCLUSTERED ([Id] ASC)
);
GO

