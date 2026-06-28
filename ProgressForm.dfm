object ProgressFrm: TProgressFrm
  Left = 359
  Top = 299
  BorderIcons = []
  BorderStyle = bsDialog
  Caption = 'Processing...'
  ClientHeight = 63
  ClientWidth = 221
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  OldCreateOrder = False
  Position = poMainFormCenter
  OnClose = FormClose
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object PercentLabel: TLabel
    Left = 176
    Top = 8
    Width = 17
    Height = 13
    Caption = '0 %'
  end
  object Button1: TButton
    Left = 80
    Top = 32
    Width = 75
    Height = 25
    Caption = '&Cancel'
    TabOrder = 0
    OnClick = Button1Click
  end
  object PB: TProgressBar
    Left = 16
    Top = 8
    Width = 150
    Height = 17
    Min = 0
    Max = 100
    TabOrder = 1
  end
end
