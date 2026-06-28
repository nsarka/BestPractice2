object BPMain: TBPMain
  Left = 223
  Top = 112
  ActiveControl = OpenFileBtn
  BorderIcons = [biSystemMenu, biHelp]
  BorderStyle = bsSingle
  Caption = 'BestPractice'
  ClientHeight = 512
  ClientWidth = 553
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Shell Dlg'
  Font.Style = []
  HelpFile = 'bestpractice.hlp'
  KeyPreview = True
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  OnCreate = FormCreate
  OnKeyUp = FormKeyUp
  OnShortCut = FormShortCut
  PixelsPerInch = 96
  TextHeight = 13
  object Shape1: TShape
    Left = 320
    Top = 176
    Width = 97
    Height = 33
    Brush.Style = bsClear
    Pen.Color = clBtnShadow
  end
  object Label5: TLabel
    Left = 10
    Top = 52
    Width = 53
    Height = 16
    HelpContext = 16
    Caption = 'Disc title:'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object Label3: TLabel
    Left = 10
    Top = 3
    Width = 45
    Height = 13
    HelpContext = 18
    Caption = 'Pick drive'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object Label6: TLabel
    Left = 8
    Top = 390
    Width = 51
    Height = 13
    HelpContext = 19
    Caption = 'Messages:'
  end
  object Bevel1: TBevel
    Left = 8
    Top = 380
    Width = 297
    Height = 9
    Shape = bsBottomLine
  end
  object Label4: TLabel
    Left = 72
    Top = 52
    Width = 4
    Height = 16
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object TimeLabel: TLabel
    Left = 327
    Top = 178
    Width = 81
    Height = 29
    Caption = '0:00:00'
    Color = clBlack
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -24
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentColor = False
    ParentFont = False
    Transparent = True
  end
  object Bevel2: TBevel
    Left = 8
    Top = 40
    Width = 297
    Height = 9
    Shape = bsBottomLine
  end
  object Label11: TLabel
    Left = 336
    Top = 20
    Width = 35
    Height = 13
    HelpContext = 8
    Caption = 'Volume'
  end
  object CueBar: TTrackBar
    Left = 314
    Top = 214
    Width = 231
    Height = 19
    HelpContext = 8
    Enabled = False
    Orientation = trHorizontal
    PageSize = 0
    Frequency = 1
    Position = 0
    SelEnd = 0
    SelStart = 0
    TabOrder = 9
    ThumbLength = 12
    TickMarks = tmBoth
    TickStyle = tsNone
  end
  object RefreshCDBtn: TButton
    Left = 16
    Top = 324
    Width = 129
    Height = 25
    HelpContext = 2
    Caption = '&Refresh CD'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 3
    OnClick = RefreshCDBtnClick
  end
  object CheckCDDB: TCheckBox
    Left = 168
    Top = 327
    Width = 129
    Height = 17
    HelpContext = 3
    Caption = 'Check C&DDB'
    Checked = True
    State = cbChecked
    TabOrder = 4
  end
  object AboutBtn: TButton
    Left = 320
    Top = 472
    Width = 97
    Height = 25
    HelpContext = 14
    Caption = '&About'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 11
    OnClick = AboutBtnClick
  end
  object TrackList: TListView
    Left = 8
    Top = 75
    Width = 297
    Height = 212
    HelpContext = 16
    Columns = <
      item
        Caption = 'Track #'
        MinWidth = 60
        Width = 60
      end
      item
        Caption = 'Track Title'
        Width = 214
      end>
    ReadOnly = True
    RowSelect = True
    PopupMenu = TrackListMenu
    StateImages = ImList
    TabOrder = 1
    ViewStyle = vsReport
    OnDblClick = TrackListDblClick
  end
  object DriveList: TComboBox
    Left = 8
    Top = 20
    Width = 297
    Height = 21
    HelpContext = 17
    Style = csDropDownList
    ItemHeight = 13
    TabOrder = 0
  end
  object HelpBtn: TButton
    Left = 424
    Top = 472
    Width = 97
    Height = 25
    HelpContext = 15
    Caption = '&Help'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 12
    OnClick = HelpBtnClick
  end
  object PageControl1: TPageControl
    Left = 312
    Top = 240
    Width = 233
    Height = 225
    ActivePage = TabSheet2
    HotTrack = True
    TabIndex = 1
    TabOrder = 10
    object TabSheet1: TTabSheet
      HelpContext = 19
      Caption = 'T&ime and Pitch'
      object Label2: TLabel
        Left = 9
        Top = 99
        Width = 66
        Height = 13
        HelpContext = 11
        Caption = 'Playing speed'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object Label1: TLabel
        Left = 170
        Top = 136
        Width = 39
        Height = 13
        HelpContext = 11
        Alignment = taRightJustify
        Caption = '100.0%'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object Label7: TLabel
        Left = 10
        Top = 175
        Width = 97
        Height = 13
        HelpContext = 13
        Caption = 'Time-stretch Quality'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object Label8: TLabel
        Left = 9
        Top = -1
        Width = 60
        Height = 13
        HelpContext = 9
        Caption = 'Playing pitch'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object PitchLbl: TLabel
        Left = 144
        Top = 35
        Width = 65
        Height = 13
        HelpContext = 9
        Alignment = taRightJustify
        Caption = '0 semitone(s)'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object Label9: TLabel
        Left = 9
        Top = 52
        Width = 77
        Height = 13
        HelpContext = 10
        Caption = 'Pitch fine adjust'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object FineLbl: TLabel
        Left = 173
        Top = 88
        Width = 35
        Height = 13
        HelpContext = 10
        Alignment = taRightJustify
        Caption = '0 cents'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object SpeedCtrl: TScrollBar
        Left = 8
        Top = 117
        Width = 199
        Height = 17
        HelpContext = 11
        LargeChange = 10
        Max = 2000
        Min = 200
        PageSize = 0
        Position = 1000
        TabOrder = 0
        OnChange = SpeedCtrlChange
      end
      object AlgoBox: TComboBox
        Left = 133
        Top = 172
        Width = 90
        Height = 21
        HelpContext = 13
        Style = csDropDownList
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ItemHeight = 13
        ItemIndex = 2
        ParentFont = False
        TabOrder = 7
        Text = 'Best so far...'
        Items.Strings = (
          'Low'
          'Intermediate'
          'Best so far...')
      end
      object FineCtrl: TScrollBar
        Left = 8
        Top = 69
        Width = 199
        Height = 17
        HelpContext = 10
        Max = 50
        Min = -50
        PageSize = 0
        TabOrder = 2
        OnChange = FineCtrlChange
      end
      object PitchCtrl: TScrollBar
        Left = 8
        Top = 16
        Width = 199
        Height = 17
        HelpContext = 9
        Max = 12
        Min = -12
        PageSize = 0
        TabOrder = 1
        OnChange = PitchCtrlChange
      end
      object FilterBox: TCheckBox
        Left = 11
        Top = 149
        Width = 206
        Height = 17
        HelpContext = 12
        Caption = 'Anti-aliasing filter'
        Checked = True
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        State = cbChecked
        TabOrder = 6
      end
      object BitBtn1: TBitBtn
        Left = 210
        Top = 16
        Width = 15
        Height = 15
        HelpContext = 29
        TabOrder = 3
        OnClick = BitBtn1Click
        Glyph.Data = {
          92000000424D9200000000000000760000002800000006000000070000000100
          0400000000001C000000120B0000120B00001000000000000000000000000000
          8000008000000080800080000000800080008080000080808000C0C0C0000000
          FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00FFFFFF00FFFF
          FF009FFF9F00F9F9FF00FF9FFF00F9F9FF009FFF9F00}
      end
      object BitBtn2: TBitBtn
        Left = 210
        Top = 70
        Width = 15
        Height = 15
        HelpContext = 29
        TabOrder = 4
        OnClick = BitBtn2Click
        Glyph.Data = {
          92000000424D9200000000000000760000002800000006000000070000000100
          0400000000001C000000120B0000120B00001000000000000000000000000000
          8000008000000080800080000000800080008080000080808000C0C0C0000000
          FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00FFFFFF00FFFF
          FF009FFF9F00F9F9FF00FF9FFF00F9F9FF009FFF9F00}
      end
      object BitBtn3: TBitBtn
        Left = 210
        Top = 118
        Width = 15
        Height = 15
        HelpContext = 29
        TabOrder = 5
        OnClick = BitBtn3Click
        Glyph.Data = {
          92000000424D9200000000000000760000002800000006000000070000000100
          0400000000001C000000120B0000120B00001000000000000000000000000000
          8000008000000080800080000000800080008080000080808000C0C0C0000000
          FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00FFFFFF00FFFF
          FF009FFF9F00F9F9FF00FF9FFF00F9F9FF009FFF9F00}
      end
    end
    object TabSheet2: TTabSheet
      HelpContext = 20
      Caption = '&Karaoke'
      ImageIndex = 1
      object BassLabel: TLabel
        Left = 172
        Top = 103
        Width = 22
        Height = 13
        Caption = '0 Hz'
      end
      object Label12: TLabel
        Left = 6
        Top = 54
        Width = 18
        Height = 13
        Caption = 'Left'
      end
      object Label13: TLabel
        Left = 180
        Top = 54
        Width = 25
        Height = 13
        Caption = 'Right'
      end
      object HighLabel: TLabel
        Left = 172
        Top = 154
        Width = 22
        Height = 13
        Caption = '0 Hz'
      end
      object Label14: TLabel
        Left = 6
        Top = 36
        Width = 157
        Height = 13
        Caption = 'Slide for optimal vocal suppresion'
      end
      object Label16: TLabel
        Left = 6
        Top = 85
        Width = 167
        Height = 13
        Caption = 'Bass pass-through frequency range'
      end
      object Label17: TLabel
        Left = 6
        Top = 136
        Width = 174
        Height = 13
        Caption = 'Treble pass-through frequency range'
      end
      object KaraokeBox: TCheckBox
        Left = 4
        Top = 12
        Width = 97
        Height = 17
        HelpContext = 21
        Alignment = taLeftJustify
        Caption = 'Karaoke &mode'
        TabOrder = 0
      end
      object BassBar: TScrollBar
        Left = 8
        Top = 103
        Width = 161
        Height = 17
        HelpContext = 23
        Enabled = False
        Max = 500
        PageSize = 0
        TabOrder = 2
        OnChange = BassBarChange
      end
      object PositionBar: TScrollBar
        Left = 48
        Top = 54
        Width = 129
        Height = 17
        HelpContext = 22
        LargeChange = 64
        Max = 256
        Min = 1
        PageSize = 0
        Position = 128
        SmallChange = 16
        TabOrder = 1
        OnChange = PositionBarChange
      end
      object HighBar: TScrollBar
        Left = 8
        Top = 154
        Width = 161
        Height = 17
        HelpContext = 24
        Enabled = False
        Max = 2000
        PageSize = 0
        TabOrder = 3
        OnChange = HighBarChange
      end
    end
  end
  object Messages: TListView
    Left = 8
    Top = 408
    Width = 297
    Height = 89
    Columns = <
      item
        Caption = 'Time'
        MinWidth = 50
        Width = 60
      end
      item
        Caption = 'Message'
        Width = 215
      end>
    TabOrder = 5
    ViewStyle = vsReport
  end
  object OpenFileBtn: TButton
    Left = 16
    Top = 356
    Width = 129
    Height = 25
    HelpContext = 6
    Caption = '&Open File...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 2
    OnClick = OpenFileClick
  end
  object GrpLoopControls: TGroupBox
    Left = 320
    Top = 48
    Width = 225
    Height = 103
    Caption = 'Loop Controls'
    Enabled = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 8
    object Label15: TLabel
      Left = 80
      Top = 30
      Width = 4
      Height = 13
      Caption = ':'
    end
    object Label18: TLabel
      Left = 128
      Top = 30
      Width = 4
      Height = 13
      Caption = ':'
    end
    object Label19: TLabel
      Left = 80
      Top = 58
      Width = 4
      Height = 13
      Caption = ':'
    end
    object Label20: TLabel
      Left = 128
      Top = 58
      Width = 4
      Height = 13
      Caption = ':'
    end
    object Start: TLabel
      Left = 8
      Top = 34
      Width = 24
      Height = 13
      Caption = 'Start'
    end
    object End: TLabel
      Left = 8
      Top = 58
      Width = 18
      Height = 13
      Caption = 'End'
    end
    object Label21: TLabel
      Left = 40
      Top = 16
      Width = 16
      Height = 13
      Caption = 'min'
    end
    object Label22: TLabel
      Left = 88
      Top = 16
      Width = 16
      Height = 13
      Caption = 'sec'
    end
    object Label23: TLabel
      Left = 136
      Top = 16
      Width = 36
      Height = 13
      Caption = '1/100 s'
    end
    object LoopStartBtn: TButton
      Left = 178
      Top = 26
      Width = 41
      Height = 25
      HelpContext = 7
      Caption = '&Now!'
      Enabled = False
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 6
      OnClick = LoopStartBtnClick
    end
    object LoopEndBtn: TButton
      Left = 178
      Top = 54
      Width = 41
      Height = 25
      HelpContext = 7
      Caption = 'No&w!'
      Enabled = False
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 13
      OnClick = LoopEndBtnClick
    end
    object EdtLoopStartMin: TEdit
      Left = 38
      Top = 29
      Width = 25
      Height = 21
      HelpContext = 28
      ReadOnly = True
      TabOrder = 0
      Text = '0'
      OnExit = EdtLoopStartMinExit
    end
    object EdtLoopStartFrame: TEdit
      Left = 134
      Top = 29
      Width = 25
      Height = 21
      HelpContext = 28
      MaxLength = 2
      ReadOnly = True
      TabOrder = 4
      Text = '0'
    end
    object EdtLoopStartSec: TEdit
      Left = 86
      Top = 29
      Width = 25
      Height = 21
      HelpContext = 28
      ReadOnly = True
      TabOrder = 2
      Text = '0'
    end
    object UDLoopStartFrame: TUpDown
      Left = 159
      Top = 29
      Width = 17
      Height = 21
      HelpContext = 28
      Associate = EdtLoopStartFrame
      Min = 0
      Max = 99
      Position = 0
      TabOrder = 5
      Wrap = True
      OnChangingEx = UDLoopStartFrameChangingEx
    end
    object UDLoopStartMin: TUpDown
      Left = 63
      Top = 29
      Width = 16
      Height = 21
      HelpContext = 28
      Associate = EdtLoopStartMin
      Min = 0
      Max = 32767
      Position = 0
      TabOrder = 1
      Wrap = False
      OnChangingEx = UDLoopStartMinChangingEx
    end
    object UDLoopStartSec: TUpDown
      Left = 111
      Top = 29
      Width = 17
      Height = 21
      HelpContext = 28
      Associate = EdtLoopStartSec
      Min = 0
      Max = 59
      Position = 0
      TabOrder = 3
      Wrap = True
      OnChangingEx = UDLoopStartSecChangingEx
    end
    object EdtLoopEndMin: TEdit
      Left = 38
      Top = 57
      Width = 25
      Height = 21
      HelpContext = 28
      ReadOnly = True
      TabOrder = 7
      Text = '0'
    end
    object UDLoopEndMin: TUpDown
      Left = 63
      Top = 57
      Width = 16
      Height = 21
      HelpContext = 28
      Associate = EdtLoopEndMin
      Min = 0
      Max = 32767
      Position = 0
      TabOrder = 8
      Wrap = False
      OnChangingEx = UDLoopEndMinChangingEx
    end
    object EdtLoopEndSec: TEdit
      Left = 86
      Top = 57
      Width = 25
      Height = 21
      HelpContext = 28
      ReadOnly = True
      TabOrder = 9
      Text = '0'
    end
    object UDLoopEndSec: TUpDown
      Left = 111
      Top = 57
      Width = 17
      Height = 21
      HelpContext = 28
      Associate = EdtLoopEndSec
      Min = 0
      Max = 59
      Position = 0
      TabOrder = 10
      Wrap = True
      OnChangingEx = UDLoopEndSecChangingEx
    end
    object EdtLoopEndFrame: TEdit
      Left = 134
      Top = 57
      Width = 25
      Height = 21
      HelpContext = 28
      ReadOnly = True
      TabOrder = 11
      Text = '0'
    end
    object UDLoopEndFrame: TUpDown
      Left = 159
      Top = 57
      Width = 17
      Height = 21
      HelpContext = 28
      Associate = EdtLoopEndFrame
      Min = 0
      Max = 99
      Position = 0
      TabOrder = 12
      Wrap = True
      OnChangingEx = UDLoopEndFrameChangingEx
    end
    object LoopCheck: TCheckBox
      Left = 8
      Top = 81
      Width = 97
      Height = 17
      HelpContext = 27
      Caption = '&Loop'
      TabOrder = 14
      OnClick = LoopCheckClick
    end
  end
  object SaveBtn: TButton
    Left = 168
    Top = 356
    Width = 129
    Height = 25
    HelpContext = 25
    Caption = '&Save to file...'
    Enabled = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 6
    OnClick = SaveBtnClick
  end
  object VolumeBar: TScrollBar
    Left = 392
    Top = 16
    Width = 145
    Height = 17
    HelpContext = 26
    LargeChange = 16
    Max = 255
    PageSize = 0
    TabOrder = 7
    OnChange = VolumeBarChange
  end
  object LangBtn: TButton
    Left = 432
    Top = 180
    Width = 105
    Height = 25
    HelpContext = 30
    Caption = 'Lan&guage'
    TabOrder = 13
    OnClick = LangBtnClick
  end
  object PrevBtn: TBitBtn
    Left = 168
    Top = 292
    Width = 57
    Height = 25
    HelpContext = 31
    Enabled = False
    TabOrder = 14
    OnClick = PrevBtnClick
    Glyph.Data = {
      DE000000424DDE0000000000000076000000280000000E0000000D0000000100
      0400000000006800000011170000111700001000000000000000000000000000
      8000008000000080800080000000800080008080000080808000C0C0C0000000
      FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00FFFFFFFFFFFF
      FF00FF88FFFFFFF88F00F008FFFFFF008F00F008FFFF80008F00F008FFF00000
      8F00F008F80000008F00F008000000008F00F008000000008F00F008FF000000
      8F00F008FFF000008F00F008FFFFF0008F00F00FFFFFFF00FF00FFFFFFFFFFFF
      FF00}
  end
  object NextBtn: TBitBtn
    Left = 240
    Top = 292
    Width = 57
    Height = 25
    HelpContext = 32
    Enabled = False
    TabOrder = 15
    OnClick = NextBtnClick
    Glyph.Data = {
      DE000000424DDE0000000000000076000000280000000E0000000D0000000100
      0400000000006800000011170000111700001000000000000000000000000000
      8000008000000080800080000000800080008080000080808000C0C0C0000000
      FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00FFFFFFFFFFFF
      FF00FF88FFFFFFF88F00F0088FFFFF008F00F000888FFF008F00F0000088FF00
      8F00F000000888008F00F000000008008F00F00000000F008F00F000000FFF00
      8F00F00000FFFF008F00F000FFFFFF008F00F00FFFFFFF00FF00FFFFFFFFFFFF
      FF00}
  end
  object SelectTrackBtn: TBitBtn
    Left = 16
    Top = 292
    Width = 57
    Height = 25
    HelpContext = 4
    Caption = 'CD'
    Enabled = False
    TabOrder = 16
    OnClick = SelectTrackBtnClick
    Glyph.Data = {
      0A020000424D0A0200000000000036000000280000000B0000000D0000000100
      180000000000D401000011170000111700000000000000000000FFFFFFFFFFFF
      C2C2C2C2C2C2FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFF
      FF000000000000C2C2C2C2C2C2FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00
      0000FFFFFF000000000000000000C2C2C2C2C2C2C2C2C2FFFFFFFFFFFFFFFFFF
      FFFFFF000000FFFFFF000000000000000000000000000000C2C2C2C2C2C2FFFF
      FFFFFFFFFFFFFF000000FFFFFF000000000000000000000000000000000000C2
      C2C2C2C2C2C2C2C2FFFFFF000000FFFFFF000000000000000000000000000000
      000000000000000000C2C2C2FFFFFF000000FFFFFF0000000000000000000000
      00000000000000000000000000FFFFFFFFFFFF000000FFFFFF00000000000000
      0000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF000000
      000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFF
      FF000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00
      0000FFFFFF000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFF000000}
  end
  object PauseBtn: TBitBtn
    Left = 88
    Top = 292
    Width = 57
    Height = 25
    HelpContext = 5
    Enabled = False
    TabOrder = 17
    OnClick = PauseBtnClick
  end
  object OpenDialog: TOpenDialog
    DefaultExt = 'wav'
    Filter = 
      'All Supported files|*.wav;*.mp3|Wave files (*.wav)|*.wav|MP3 fil' +
      'es (*.mp3)|*.mp3'
    Options = [ofHideReadOnly, ofPathMustExist, ofFileMustExist, ofEnableSizing]
    Left = 120
    Top = 32
  end
  object ApplicationEvents1: TApplicationEvents
    OnHelp = ApplicationEvents1Help
    Left = 184
    Top = 32
  end
  object SaveDialog: TSaveDialog
    DefaultExt = 'wav'
    Filter = 'Wave file (*.wav)|*.wav'
    Options = [ofHideReadOnly, ofPathMustExist, ofEnableSizing]
    Left = 152
    Top = 32
  end
  object ThemeManager1: TThemeManager
    Left = 280
    Top = 32
  end
  object ImList: TImageList
    Left = 248
    Top = 32
    Bitmap = {
      494C010103000400040010001000FFFFFFFFFF10FFFFFFFFFFFFFFFF424D3600
      0000000000003600000028000000400000001000000001002000000000000010
      000000000000000000000000000000000000FFFFFF00FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00DEEFCE00C6DEA500C6D6A500DEE7C600FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF00FFFFFF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000FFFFFF00FFFFFF00FFFFFF00FFFF
      FF00A5C66B005A8C00005A9400006394000073A5080073A50800528C0000A5BD
      6300FFFFFF00FFFFFF00FFFFFF00FFFFFF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000FFFFFF00FFFFFF00FFFFFF006394
      00006394000073A508007BA510007BAD100084AD2100A5C65200BDD67B0084B5
      29005A8C0000FFFFFF00FFFFFF00FFFFFF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000FFFFFF00FFFFFF00639400006B9C
      00007BAD100084B5180084B5180084B5210084B518008CBD3100A5CE5A00C6E7
      8C00A5C65A005A8C0000FFFFFF00FFFFFF000000000000000000000000000000
      0000CCCCCC00CCCCCC00CCCCCC000000000000000000CCCCCC00CCCCCC00CCCC
      CC00000000000000000000000000000000000000000000000000000000000000
      0000CCCCCC00CCCCCC00CCCCCC000000000000000000CCCCCC00CCCCCC00CCCC
      CC00000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000FFFFFF00A5C66B006B9C00007BAD
      18008CBD21006384180084AD210094C6290094C629008CBD21009CC64200ADD6
      6300D6EF9C0084AD2100A5BD6300FFFFFF000000000000000000000000000000
      00000000000000000000CCCCCC0000000000000000000000000000000000CCCC
      CC0000000000000000000000000000000000000000000000000000000000008F
      FE00008FFE00008FFE00CCCCCC0000000000008FFE00008FFE00008FFE00CCCC
      CC00000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000FFFFFF005A8C00007BAD100094C6
      21001018000029390800526B1800637B21007BA5290094C631009CCE2900ADD6
      5200B5DE7300CEE79400528C0000FFFFFF000000000000000000000000000000
      00000000000000000000CCCCCC0000000000000000000000000000000000CCCC
      CC0000000000000000000000000000000000000000000000000000000000008F
      FE00008FFE00008FFE00CCCCCC0000000000008FFE00008FFE00008FFE00CCCC
      CC00000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000DEEFCE006394000084B51800A5D6
      310000000000000000000000000010180800395218005A7B21006B8C210084AD
      2900A5CE5A00C6E78C006B9C0000DEE7C6000000000000000000000000000000
      00000000000000000000CCCCCC0000000000000000000000000000000000CCCC
      CC0000000000000000000000000000000000000000000000000000000000008F
      FE00008FFE00008FFE00CCCCCC0000000000008FFE00008FFE00008FFE00CCCC
      CC00000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000C6DEA5006B9C00008CBD2100ADE7
      3900000000000000000000000000000000000000000000000000182108005273
      18009CC63900B5DE6B0084AD2100C6D69C000000000000000000000000000000
      00000000000000000000CCCCCC0000000000000000000000000000000000CCCC
      CC0000000000000000000000000000000000000000000000000000000000008F
      FE00008FFE00008FFE00CCCCCC0000000000008FFE00008FFE00008FFE00CCCC
      CC00000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000C6DEA50073A5000094C62900B5EF
      4200000000000000000000000000000000000000000000000000526321009CC6
      4200A5D63900A5CE52007BAD1800C6D6A5000000000000000000000000000000
      00000000000000000000CCCCCC0000000000000000000000000000000000CCCC
      CC0000000000000000000000000000000000000000000000000000000000008F
      FE00008FFE00008FFE00CCCCCC0000000000008FFE00008FFE00008FFE00CCCC
      CC00000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000DEE7CE006B9C000094C62900BDF7
      4A00000000000000000000000000394A18009CC64A00DEFF6B00CEFF5A00BDEF
      5200ADD6420094C629006B9C0000DEE7CE000000000000000000000000000000
      00000000000000000000CCCCCC0000000000000000000000000000000000CCCC
      CC0000000000000000000000000000000000000000000000000000000000008F
      FE00008FFE00008FFE00CCCCCC0000000000008FFE00008FFE00008FFE00CCCC
      CC00000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000FFFFFF006394000094C62900BDF7
      4A00182108007B9C3900DEFF6B00DEFF6B00D6FF6B00CEFF6300CEFF6300BDF7
      5200ADDE42008CBD290063940000FFFFFF000000000000000000000000000000
      00000000000000000000CCCCCC0000000000000000000000000000000000CCCC
      CC0000000000000000000000000000000000000000000000000000000000008F
      FE00008FFE00008FFE00CCCCCC0000000000008FFE00008FFE00008FFE00CCCC
      CC00000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000FFFFFF00A5C66B007BAD0800ADDE
      4200D6FF6B00D6FF6B00CEFF6300CEFF5A00CEFF5A00CEFF6300CEFF6300C6F7
      5A00A5D642007BAD0800A5C66B00FFFFFF000000000000000000000000000000
      00000000000000000000CCCCCC0000000000000000000000000000000000CCCC
      CC0000000000000000000000000000000000000000000000000000000000008F
      FE00008FFE00008FFE00CCCCCC0000000000008FFE00008FFE00008FFE00CCCC
      CC00000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000FFFFFF00FFFFFF006B9C000094C6
      3100EFFFCE00EFFFC600EFFFBD00EFFFBD00E7FFBD00E7FFB500EFFFBD00E7F7
      B5008CBD21006B9C0000FFFFFF00FFFFFF000000000000000000000000000000
      00000000000000000000CCCCCC0000000000000000000000000000000000CCCC
      CC0000000000000000000000000000000000000000000000000000000000008F
      FE00008FFE00008FFE00CCCCCC0000000000008FFE00008FFE00008FFE00CCCC
      CC00000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000FFFFFF00FFFFFF00FFFFFF006B9C
      00008CBD2900EFF7D600FFFFFF00FFFFFF00FFFFFF00FFFFFF00E7F7C60084BD
      18006B9C0000FFFFFF00FFFFFF00FFFFFF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000000000000000000000000000008F
      FE00008FFE00008FFE000000000000000000008FFE00008FFE00008FFE000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000FFFFFF00FFFFFF00FFFFFF00FFFF
      FF00A5C66B00639C00007BB500009CC6390094C6390073AD0000639C0000A5C6
      6B00FFFFFF00FFFFFF00FFFFFF00FFFFFF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000FFFFFF00FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00DEE7C600C6D69C00C6D69C00DEE7CE00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF00FFFFFF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000424D3E000000000000003E000000
      2800000040000000100000000100010000000000800000000000000000000000
      000000000000000000000000FFFFFF000000FFFFFFFF00000000FFFFFFFF0000
      0000FFFFFFFF00000000F18FF18F00000000E10FE10F00000000E10FE10F0000
      0000E10FE10F00000000E10FE10F00000000E10FE10F00000000E10FE10F0000
      0000E10FE10F00000000E10FE10F00000000E10FE10F00000000E31FE31F0000
      0000FFFFFFFF00000000FFFFFFFF000000000000000000000000000000000000
      000000000000}
  end
  object TrackListMenu: TPopupMenu
    Images = ImList
    OnPopup = TrackListMenuPopup
    Left = 216
    Top = 32
    object Selplaytrack: TMenuItem
      Caption = '&Play CD track'
      Default = True
      ImageIndex = 0
      OnClick = SelplaytrackClick
    end
    object RefreshCD1: TMenuItem
      Caption = '&Refresh CD'
      OnClick = RefreshCD1Click
    end
  end
end
