object ILangForm: TILangForm
  Left = 331
  Top = 262
  Width = 318
  Height = 298
  Caption = 'Choose Language'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object LVLangs: TListView
    Left = 8
    Top = 8
    Width = 297
    Height = 201
    Columns = <
      item
        Caption = 'Language'
        MinWidth = 200
        Width = 200
      end
      item
        Caption = 'Code'
      end>
    RowSelect = True
    TabOrder = 0
    ViewStyle = vsReport
    OnDblClick = LVLangsDblClick
  end
  object OKBtn: TButton
    Left = 24
    Top = 224
    Width = 113
    Height = 25
    Caption = 'Select &Language'
    ModalResult = 1
    TabOrder = 1
    OnClick = OKBtnClick
  end
  object Button2: TButton
    Left = 176
    Top = 224
    Width = 115
    Height = 25
    Caption = '&Cancel'
    TabOrder = 2
    OnClick = Button2Click
  end
end
