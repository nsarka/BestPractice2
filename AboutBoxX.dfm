object AboutX: TAboutX
  Left = 386
  Top = 231
  BorderStyle = bsNone
  Caption = 'About BestPractice'
  ClientHeight = 163
  ClientWidth = 367
  Color = clWhite
  DefaultMonitor = dmMainForm
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poMainFormCenter
  Scaled = False
  OnClick = FormClick
  OnKeyDown = FormKeyDown
  OnPaint = FormPaint
  PixelsPerInch = 96
  TextHeight = 13
  object Shape1: TShape
    Left = 0
    Top = 0
    Width = 367
    Height = 163
    Align = alClient
    Brush.Style = bsClear
    Pen.Color = clGray
  end
  object Label1: TLabel
    Left = 152
    Top = 120
    Width = 137
    Height = 13
    Cursor = crHandPoint
    Caption = 'http://akrip.sourceforge.net'
    Font.Charset = ANSI_CHARSET
    Font.Color = 15632520
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    OnClick = Label1Click
  end
  object Label3: TLabel
    Left = 352
    Top = 2
    Width = 7
    Height = 13
    Cursor = crHandPoint
    Caption = 'X'
    OnClick = Label3Click
  end
  object Label2: TLabel
    Left = 152
    Top = 76
    Width = 173
    Height = 13
    Cursor = crHandPoint
    Caption = 'http://bestpractice.sourceforge.net'
    Font.Charset = ANSI_CHARSET
    Font.Color = 15632520
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    OnClick = Label1Click
  end
  object TitleLbl: TLabel
    Left = 153
    Top = 23
    Width = 102
    Height = 19
    Caption = 'BestPractice'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 4473924
    Font.Height = -16
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
    Transparent = True
  end
  object AKRipLbl: TLabel
    Left = 152
    Top = 104
    Width = 40
    Height = 13
    Caption = 'AKTITLE'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 4473924
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object Label4: TLabel
    Left = 152
    Top = 48
    Width = 123
    Height = 13
    Caption = '(c) 2007 Robert Moerland'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 4473924
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object Label5: TLabel
    Left = 152
    Top = 62
    Width = 162
    Height = 13
    Caption = 'Released under the GPL version 2'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 4473924
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
end
