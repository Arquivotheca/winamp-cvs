VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "Mscomctl.ocx"
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "Comdlg32.ocx"
Begin VB.Form MainForm 
   AutoRedraw      =   -1  'True
   BorderStyle     =   1  'Fixed Single
   Caption         =   "VB Sample for PrimoSDK"
   ClientHeight    =   4455
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   9330
   ClipControls    =   0   'False
   Icon            =   "Sample_VB.frx":0000
   KeyPreview      =   -1  'True
   LinkTopic       =   "Form1"
   LockControls    =   -1  'True
   MaxButton       =   0   'False
   ScaleHeight     =   4455
   ScaleWidth      =   9330
   StartUpPosition =   2  'CenterScreen
   Begin VB.ComboBox cmbSource 
      Height          =   315
      Left            =   870
      Style           =   2  'Dropdown List
      TabIndex        =   40
      Top             =   2610
      Width           =   3645
   End
   Begin VB.CommandButton cmdEjectSource 
      Caption         =   "Eject"
      Height          =   375
      Left            =   870
      TabIndex        =   39
      Top             =   2970
      Width           =   705
   End
   Begin VB.CommandButton cmdCloseSource 
      Caption         =   "Close"
      Height          =   375
      Left            =   1620
      TabIndex        =   38
      Top             =   2970
      Width           =   705
   End
   Begin VB.CommandButton cmdDriveInfoSource 
      Caption         =   "Drive I&nfo"
      Height          =   375
      Left            =   2550
      TabIndex        =   37
      Top             =   2970
      Width           =   885
   End
   Begin VB.CommandButton cmdDiscInfoSource 
      Caption         =   "Disc In&fo"
      Height          =   375
      Left            =   3630
      TabIndex        =   36
      Top             =   2970
      Width           =   885
   End
   Begin VB.ListBox lstFileList 
      Height          =   1365
      IntegralHeight  =   0   'False
      Left            =   60
      OLEDragMode     =   1  'Automatic
      OLEDropMode     =   1  'Manual
      TabIndex        =   12
      Top             =   3000
      Width           =   4435
   End
   Begin VB.CommandButton cmdClearData 
      Caption         =   "Clear List"
      Height          =   345
      Left            =   3540
      TabIndex        =   11
      Top             =   2580
      Width           =   975
   End
   Begin MSComDlg.CommonDialog cdaDialog 
      Left            =   6420
      Top             =   3960
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.Timer ProgTimer 
      Enabled         =   0   'False
      Interval        =   1000
      Left            =   6990
      Top             =   3960
   End
   Begin VB.PictureBox Picture2 
      Appearance      =   0  'Flat
      BorderStyle     =   0  'None
      ForeColor       =   &H80000008&
      Height          =   495
      Left            =   7470
      ScaleHeight     =   495
      ScaleWidth      =   1155
      TabIndex        =   34
      TabStop         =   0   'False
      Top             =   3570
      Width           =   1155
      Begin VB.OptionButton optMode1 
         Alignment       =   1  'Right Justify
         Caption         =   "Mode 1"
         Height          =   195
         Left            =   0
         TabIndex        =   20
         Top             =   0
         Visible         =   0   'False
         Width           =   945
      End
      Begin VB.OptionButton optMode2 
         Alignment       =   1  'Right Justify
         Caption         =   "Mode 2"
         Height          =   285
         Left            =   0
         TabIndex        =   21
         Top             =   210
         Visible         =   0   'False
         Width           =   945
      End
   End
   Begin VB.PictureBox PicExtract 
      Appearance      =   0  'Flat
      BorderStyle     =   0  'None
      ForeColor       =   &H80000008&
      Height          =   405
      Left            =   1710
      ScaleHeight     =   405
      ScaleWidth      =   2745
      TabIndex        =   32
      TabStop         =   0   'False
      Top             =   3360
      Width           =   2745
      Begin VB.TextBox txtEditTrack 
         Height          =   285
         Left            =   2340
         TabIndex        =   26
         Text            =   "1"
         Top             =   90
         Width           =   345
      End
      Begin VB.Label lblTrackExtract 
         Caption         =   "Track to extract as a Wave file:"
         Height          =   195
         Left            =   60
         TabIndex        =   33
         Top             =   120
         Width           =   2265
      End
   End
   Begin VB.PictureBox Picture1 
      AutoSize        =   -1  'True
      Height          =   540
      Left            =   8715
      Picture         =   "Sample_VB.frx":57E2
      ScaleHeight     =   480
      ScaleWidth      =   480
      TabIndex        =   22
      TabStop         =   0   'False
      Top             =   3840
      Width           =   540
   End
   Begin MSComctlLib.Slider sliSwap 
      Height          =   375
      Left            =   5400
      TabIndex        =   16
      Top             =   3510
      Visible         =   0   'False
      Width           =   1095
      _ExtentX        =   1931
      _ExtentY        =   661
      _Version        =   393216
      LargeChange     =   1
      Max             =   257
      TickStyle       =   3
   End
   Begin VB.OptionButton optUDF 
      Alignment       =   1  'Right Justify
      Caption         =   "UDF (Bridge)"
      Height          =   255
      Left            =   7890
      TabIndex        =   19
      Top             =   3120
      Visible         =   0   'False
      Width           =   1335
   End
   Begin VB.OptionButton optJoliet 
      Alignment       =   1  'Right Justify
      Caption         =   "Microsoft® Joliet"
      Height          =   225
      Left            =   7710
      TabIndex        =   18
      Top             =   2880
      Visible         =   0   'False
      Width           =   1515
   End
   Begin VB.OptionButton optIsoLevel 
      Alignment       =   1  'Right Justify
      Caption         =   "ISO-9660 Level 1"
      Height          =   285
      Left            =   7650
      TabIndex        =   17
      Top             =   2580
      Visible         =   0   'False
      Width           =   1575
   End
   Begin VB.CheckBox chkCloseDisc 
      Alignment       =   1  'Right Justify
      Caption         =   "Close Disc"
      Height          =   285
      Left            =   6240
      TabIndex        =   15
      Top             =   3000
      Visible         =   0   'False
      Width           =   1095
   End
   Begin VB.TextBox txtload 
      Height          =   315
      Left            =   5730
      TabIndex        =   14
      Text            =   "0"
      Top             =   2970
      Visible         =   0   'False
      Width           =   285
   End
   Begin VB.TextBox txtVOLUMENAME 
      Height          =   285
      Left            =   5730
      MaxLength       =   11
      TabIndex        =   13
      Top             =   2610
      Visible         =   0   'False
      Width           =   1605
   End
   Begin VB.TextBox txtResult 
      BackColor       =   &H8000000F&
      Height          =   1905
      Left            =   4740
      Locked          =   -1  'True
      MultiLine       =   -1  'True
      ScrollBars      =   2  'Vertical
      TabIndex        =   27
      TabStop         =   0   'False
      Top             =   420
      Width           =   4545
   End
   Begin MSComctlLib.ProgressBar prbProgress 
      Height          =   315
      Left            =   4740
      TabIndex        =   25
      Top             =   30
      Width           =   4545
      _ExtentX        =   8017
      _ExtentY        =   556
      _Version        =   393216
      Appearance      =   1
      Scrolling       =   1
   End
   Begin VB.CheckBox chkRecToAll 
      Caption         =   "Simultaneous to &All present Drives similar to Recorder"
      Height          =   285
      Left            =   180
      TabIndex        =   10
      Top             =   2100
      Width           =   4065
   End
   Begin VB.CommandButton cmdDiscInfoRec 
      Caption         =   "&Disc Info"
      Height          =   375
      Left            =   3630
      TabIndex        =   9
      Top             =   1680
      Width           =   885
   End
   Begin VB.CommandButton cmddriveInfoRec 
      Caption         =   "Dri&ve Info"
      Height          =   375
      Left            =   2550
      TabIndex        =   8
      Top             =   1680
      Width           =   885
   End
   Begin VB.CommandButton cmdCloseRec 
      Caption         =   "Close"
      Height          =   375
      Left            =   1620
      TabIndex        =   7
      Top             =   1680
      Width           =   705
   End
   Begin VB.CommandButton cmdEjectRec 
      Appearance      =   0  'Flat
      Caption         =   "Eject"
      Height          =   375
      Left            =   870
      TabIndex        =   6
      Top             =   1680
      Width           =   705
   End
   Begin VB.CommandButton btnGo 
      Caption         =   "G&o !"
      Height          =   375
      Left            =   3420
      TabIndex        =   4
      Top             =   750
      Width           =   1095
   End
   Begin VB.ComboBox cmbRecorder 
      Height          =   315
      Left            =   870
      Style           =   2  'Dropdown List
      TabIndex        =   5
      Top             =   1320
      Width           =   3645
   End
   Begin VB.CheckBox chkTest 
      Alignment       =   1  'Right Justify
      Caption         =   "&Test"
      Height          =   285
      Left            =   2520
      TabIndex        =   3
      Top             =   780
      Value           =   1  'Checked
      Width           =   645
   End
   Begin VB.ComboBox cmbSpeed 
      Height          =   315
      Left            =   840
      Style           =   2  'Dropdown List
      TabIndex        =   2
      Top             =   750
      Width           =   795
   End
   Begin VB.ComboBox cmbFunction 
      Height          =   315
      Left            =   210
      Style           =   2  'Dropdown List
      TabIndex        =   1
      Top             =   330
      Width           =   4305
   End
   Begin VB.Label lblSource 
      Caption         =   "&Source:"
      Height          =   225
      Left            =   270
      TabIndex        =   41
      Top             =   2640
      Width           =   645
   End
   Begin VB.Line Line6 
      BorderColor     =   &H80000014&
      X1              =   60
      X2              =   4620
      Y1              =   1215
      Y2              =   1215
   End
   Begin VB.Line Line5 
      BorderColor     =   &H80000014&
      X1              =   60
      X2              =   9240
      Y1              =   2475
      Y2              =   2475
   End
   Begin VB.Label lblFileListString 
      Caption         =   "File/Dir, Audio or MPEG files (drag && drop here):"
      Height          =   285
      Left            =   120
      TabIndex        =   35
      Top             =   2730
      Width           =   3375
   End
   Begin VB.Label lblSwapResult 
      Caption         =   "None"
      Height          =   225
      Left            =   6570
      TabIndex        =   31
      Top             =   3540
      Visible         =   0   'False
      Width           =   465
   End
   Begin VB.Label lblswap 
      Caption         =   "Swap:"
      Height          =   225
      Left            =   4890
      TabIndex        =   30
      Top             =   3540
      Visible         =   0   'False
      Width           =   435
   End
   Begin VB.Label lblLoad 
      Caption         =   "Load:"
      Height          =   225
      Left            =   5250
      TabIndex        =   29
      Top             =   3030
      Visible         =   0   'False
      Width           =   435
   End
   Begin VB.Label lblVolume 
      Caption         =   "Volume:"
      Height          =   225
      Left            =   5100
      TabIndex        =   28
      Top             =   2670
      Visible         =   0   'False
      Width           =   615
   End
   Begin VB.Line Line2 
      BorderColor     =   &H80000010&
      X1              =   60
      X2              =   9240
      Y1              =   2460
      Y2              =   2460
   End
   Begin VB.Label lblRecorder 
      Caption         =   "Recorder:"
      Height          =   225
      Left            =   120
      TabIndex        =   24
      Top             =   1380
      Width           =   765
   End
   Begin VB.Line Line1 
      BorderColor     =   &H80000010&
      X1              =   60
      X2              =   4620
      Y1              =   1200
      Y2              =   1200
   End
   Begin VB.Label Label2 
      Caption         =   "Sp&eed:"
      Height          =   225
      Left            =   300
      TabIndex        =   23
      Top             =   810
      Width           =   525
   End
   Begin VB.Label Label1 
      Caption         =   "Operation:"
      Height          =   225
      Left            =   240
      TabIndex        =   0
      Top             =   90
      Width           =   795
   End
End
Attribute VB_Name = "MainForm"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

'********************************************************************
'*                                                                  *
'*                                                                  *
'*                      T  e  s  t  e  r  V  B                      *
'*                     ========================                     *
'*                                                                  *
'*                                                                  *
'*  Mini mastering application as a VB example and a guide to use   *
'*  the PrimoSDK API.                                               *
'*                                                                  *
'*  This computer program is protected by copyright law and         *
'*  international treaties. Unauthorized reproduction or            *
'*  distribution of this program, or any portion of it, may result  *
'*  in severe civil and criminal penalties.                         *
'*                                                                  *
'*  If you are a licensed user of PrimoSDK you are authorized to    *
'*  copy part or entirely the code of this example into your        *
'*  application.                                                    *
'*                                                                  *
'*  Copyright(C) 2002 Sonic Solutions                               *
'*                                                                  *
'********************************************************************



'
' Win32 FUNCTION DECLARES
'
Private Declare Function GetKeyState Lib "user32" (ByVal nVirtKey As Long) As Integer
Private Declare Sub Sleep Lib "kernel32" (ByVal dwMilliseconds As Long)
Private Const VK_SHIFT = &H10

'
' TESTER OPERATION CODES
'
Const FUNC_COPY = 1
Const FUNC_BUILDGI = 2
Const FUNC_RECORDGIORTRACK = 3
Const FUNC_VERIFYDISC = 4
Const FUNC_VERIFYGI = 5
Const FUNC_RECORDDATA = 6
Const FUNC_SAVEDATA = 7
Const FUNC_RECORDAUDIO = 8
Const FUNC_RECORDVIDEO = 9
Const FUNC_AUDIOEXTRACT = 10
Const FUNC_ERASEDISC = 11

'
' GLOBAL VARIABLES
'
Dim LetterSource As Long                                    ' Letter of the Source Drive
Dim CurFunc As Long                                         ' Selected Function
Dim bBusy As Boolean                                        ' Application's Status
Dim HowManyRecorder As Long                                 ' How many recorders
Dim Handle As Long                                          ' An Handle for the operations
Dim MediumType As Long                                      ' Type of
Dim MediumFormat                                            '  the disc
Dim UnitDescr As Long                                       ' Unit Vendor, Model and FW. version
Dim Tracks As Long                                          ' Total tracks number
Dim Free As Long                                            ' Total free sectors
Dim TotalSize, TotalFiles As Long                           ' Size of the image and total number of files
Dim TotEntries As Long                                      ' Lines in the files listbox
Dim bStop As Boolean                                        ' Set when user stops
Dim HowManyReader As Long                                   ' How many drives to read disc (recorders count)
Dim UnitSource As Long                                      ' Unit for reading from
Dim UnitsRec(1 To 64) As Long                               ' Units for recording
Dim WAction As Long                                         ' Test or Write
Dim Speed As Long                                           ' Speed to use
Dim TotFileCount As Long                                    ' Total Files

Private Sub Form_Load()
'
' STARTING, INIT PrimoSDK AND LOOK FOR DRIVES
'
Dim DllRelease As Long
Dim Unit As Long
Dim UnitType As Long
Dim UnitReady As Long
Dim Descr As String * 64

    ' IF SHIFT IS PRESSED WE START IN TRACE MODE
    If ((GetKeyState(VK_SHIFT) And &H8000) <> 0) Then
        PrimoSDK_Trace 1
        Caption = Caption + " (Trace mode)"
    End If
    
    '
    ' INIT PrimoSDK AND SET THE CAPTION WITH THE VERSION NUMBER
    '
    Reply = PrimoSDK_Init(DllRelease)
    Select Case Reply
    Case PRIMOSDK_CDDVDVERSION
        Caption = Caption + " for CD/DVD"
    Case PRIMOSDK_DEMOVERSION
        Caption = Caption + " Demo"
    Case Else
        DisplayError Reply, "PrimoSDK_Init", ""
        Exit Sub
    End Select
    ' SET THE APP TITLE TO THE CAPTION NOW(WHICH WILL BE ALSO EVERY MSGBOX CAPTION)
    App.Title = Caption
    ' ADD TO THE CAPTION THE PrimoSDK VERSION NUMBER
    Caption = Caption + " - PrimoSDK DLL Version " + Format$(DllRelease \ (&HFFFFFF)) + "." + Format$(((DllRelease And &HFF0000) \ 65535)) + "." + Format$((DllRelease And 65535), "000")
    
    ' GET THE ONLY HANDLE WE WILL USE IN THIS APP
    Reply = PrimoSDK_GetHandle(Handle)
    If Reply <> PRIMOSDK_OK Then
        DisplayError Reply, "PrimoSDK_GetHandle", ""
        End
    End If
    
    '
    ' LOAD THE DRIVE LETTER COMBO WITH THE EXISTING DRIVES
    '  DOING A LOOP FROM 'D' TO 'Z' AND ASKING PrimoSDK_UnitInfo
    '
    For i = &H44 To &H5A
        Unit = i
        Reply = PrimoSDK_UnitInfo(Handle, Unit, UnitType, Descr$, UnitReady)
        If (Reply = 0) Then
            '
            'ALL THE UNITS GO UNDER THE READER COMBO BOX, NO MATTER WHAT THEY ARE
            '
            Host = Unit \ 16777216
            Target = (Unit \ 65536) And 255
            LUN = (Unit \ 256) And 255
            cmbSource.AddItem Chr$(i) + ": (h" + Format$(Host) + " id" + Format$(Target) + ") " + VBStr(Descr$), HowManyReader
            cmbSource.ItemData(HowManyReader) = Unit
            HowManyReader = HowManyReader + 1
            '
            'IF IT WAS A RECORDER ADD TO THE RECORDERS COMBO, TOO
            '
            If (UnitType = PRIMOSDK_CDR) Or (UnitType = PRIMOSDK_CDRW) Or _
               (UnitType = PRIMOSDK_DVDRAM) Or (UnitType = PRIMOSDK_DVDR) Or _
               (UnitType = PRIMOSDK_DVDRW) Or (UnitType = PRIMOSDK_DVDPRW) Then
                cmbRecorder.AddItem Chr$(i) + ": (h" + Format$(Host) + " id" + Format$(Target) + ") " + VBStr(Descr$), HowManyRecorder
                cmbRecorder.ItemData(HowManyRecorder) = Unit
                HowManyRecorder = HowManyRecorder + 1
            End If
        End If
    Next i
   
    '
    ' IF NO RECORDERS, WE JUST ABORT
    '
    If (HowManyReader = 0) Then
        MsgBox "Not even a CD-ROM has been found. Why to continue?", vbInformation
        End
    End If
    ' AND A MESSAGE IF NO RCORDERS
    If (HowManyRecorder = 0) Then
        MsgBox "No recorders have been found.", vbInformation
        cmbRecorder.Enabled = False
        chkRecToAll.Enabled = False
    End If

    '
    ' ADD APPLICATION FUNCTION TO COMBO BOX
    '
    cmbFunction.AddItem "1 - Copy Disc to Disc, from Source to Recorder", 0
    cmbFunction.AddItem "2 - Build a Global Image from the Disc in Source", 1
    cmbFunction.AddItem "3 - Write a Global Image or Other Image to Recorder", 2
    cmbFunction.AddItem "4 - Verify Disc in Source against the Disc in Recorder", 3
    cmbFunction.AddItem "5 - Verify a Global Image against the Disc in Recorder", 4
    cmbFunction.AddItem "6 - Write a Data Disc to Recorder", 5
    cmbFunction.AddItem "7 - Build a Data Disc to Track Image", 6
    cmbFunction.AddItem "8 - Write an Audio Disc Compilation to Recorder", 7
    cmbFunction.AddItem "9 - Write a Video CD to Recorder", 8
    cmbFunction.AddItem "X - Extract an Audio Track from Source", 9
    cmbFunction.AddItem "E - Erase a Rewritable Disc in Recorder", 10
    

    '
    ' ADD SUPPORTES SPEED
    '
    cmbSpeed.AddItem "Max.", 0
    cmbSpeed.ItemData(0) = 0
    cmbSpeed.AddItem "1x", 1
    cmbSpeed.ItemData(1) = 1
    cmbSpeed.AddItem "2x", 2
    cmbSpeed.ItemData(2) = 2
    cmbSpeed.AddItem "4x", 3
    cmbSpeed.ItemData(3) = 4
    cmbSpeed.AddItem "6x", 4
    cmbSpeed.ItemData(4) = 6
    cmbSpeed.AddItem "8x", 5
    cmbSpeed.ItemData(5) = 8
    cmbSpeed.AddItem "10x", 6
    cmbSpeed.ItemData(6) = 10
    cmbSpeed.AddItem "12x", 7
    cmbSpeed.ItemData(7) = 12
    cmbSpeed.AddItem "16x", 8
    cmbSpeed.ItemData(8) = 16
    
    '
    'SET CONTROLS TO THEIR DEFAULT VALUE
    '
    optJoliet.Value = True
    optMode1.Value = True
    cmbFunction.ListIndex = 0
    If (cmbRecorder.ListCount > 0) Then cmbRecorder.ListIndex = 0
    cmbSource.ListIndex = 0
    cmbSpeed.ListIndex = 0

End Sub

Private Sub Form_Unload(Cancel As Integer)
'
' TRYING TO UNLOAD, CHECK IF THE APP IS RUNNING
'
    If (Not bBusy) Then
        ' CLOSING, TERMINATE PrimoSDK
        Reply = PrimoSDK_ReleaseHandle(Handle)
        If Reply <> PRIMOSDK_OK Then
            DisplayError Reply, "PrimoSDK_ReleaseHandle", ""
        End If
        Reply = PrimoSDK_End()
        If Reply <> PRIMOSDK_OK Then
            DisplayError Reply, "PrimoSDK_End", ""
        End If
    Else
        MsgBox "Stop any running Job before Closing...", vbInformation
        Cancel = -1
    End If

End Sub

Private Sub btnGo_Click()
'
' BEGIN THE PRE-RECORDING SET-UP
'
Dim InitialrecorderID As Long
Dim Unit As Long
Dim UnitType As Long
Dim UnitReady As Long
Dim RecorderName As String * 64
Dim InitialNum As Long
Dim SimilarRecorders As Boolean
Dim SelSourceID As Long
    
    lstFileList.ListIndex = -1
    
    '
    ' CHECK TO SEE IF WE ARE RUNNING, IF SO THIS IS A STOP
    '
    If bBusy Then
        bStop = True
        btnGo.Enabled = False
        Exit Sub
    End If

    '
    'CHECK THAT AT LEAST ONE RECORDER IS PRESENT IF THE FUNCTION NEEDS ONE
    '
    If ((CurFunc = FUNC_COPY Or CurFunc = FUNC_RECORDGIORTRACK Or CurFunc = FUNC_VERIFYDISC Or _
         CurFunc = FUNC_VERIFYGI Or CurFunc = FUNC_RECORDDATA Or CurFunc = FUNC_RECORDAUDIO Or _
         CurFunc = FUNC_ERASEDISC Or CurFunc = FUNC_RECORDVIDEO) And HowManyRecorder = 0) Then
        ' WE NEEDED ONE BUT NO RECORDER IS FOUND
        MsgBox "No recorders have been found.", vbInformation
        Exit Sub
    End If
        
    '
    ' IF WE NEED ANY RECODER(S) PUT INTO UnitsRec THE UNIT(S) LIST TO RECORD TO
    '
    If (CurFunc = FUNC_COPY Or CurFunc = FUNC_RECORDGIORTRACK Or CurFunc = FUNC_VERIFYDISC Or _
        CurFunc = FUNC_VERIFYGI Or CurFunc = FUNC_RECORDDATA Or CurFunc = FUNC_RECORDAUDIO Or _
        CurFunc = FUNC_ERASEDISC Or CurFunc = FUNC_RECORDVIDEO) Then
        ' WE NEED. CLEAN THE RECORDERS LIST
        For i = LBound(UnitsRec) To UBound(UnitsRec)
            UnitsRec(i) = 0
        Next i
        ' CHECK TO SEE IF "SIMULTANEOUS RECORDING" IS CHECKED
        If (chkRecToAll.Value = 1) Then
            ' ALL THE SIMILAR ONES
            InitialrecorderID = cmbRecorder.ItemData(cmbRecorder.ListIndex)
            Reply = PrimoSDK_UnitInfo(Handle, InitialrecorderID, Unitype, RecorderName, UnitReady)
            RecorderSelected = Left(VBStr(RecorderName), Len(VBStr(RecorderName)) - 4)
            'CHECK THE CONTENTS OF THE COMBO BOX TO SEE IF THERE ARE ANY SIMILAR UNITS
            k = 1
            For i = 0 To cmbRecorder.ListCount - 1
                RecorderID = cmbRecorder.ItemData(i)
                Reply = PrimoSDK_UnitInfo(Handle, RecorderID, Unitype, RecorderName, UnitReady)
                RecorderCurent = Left(VBStr(RecorderName), Len(VBStr(RecorderName)) - 4)
                If (RecorderSelected = RecorderCurent) Then
                    'MATCH FOUND, ADD ALL RECORDERS SIMILAR TO ORIGINAL
                    UnitsRec(k) = RecorderID
                    k = k + 1
                End If
            Next i
            UnitsRec(k) = &HFFFFFFFF
        Else
            ' ONLY THE SELECTED
            InitialrecorderID = cmbRecorder.ItemData(cmbRecorder.ListIndex)
            UnitsRec(1) = InitialrecorderID
            UnitsRec(2) = &HFFFFFFFF
        End If
    End If
        
    '
    ' IF WE NEED A SOURCE DRIVE ADD IT IN
    '
    If (CurFunc = FUNC_COPY Or CurFunc = FUNC_BUILDGI Or CurFunc = FUNC_VERIFYDISC Or _
        CurFunc = FUNC_AUDIOEXTRACT) Then
        UnitSource = cmbSource.ItemData(cmbSource.ListIndex)
    End If
        
    '
    'SET A GLOBAL VARIABLE TO SEE IF IT WAS A TEST OR REAL RECORDING
    '
    
    If (chkTest.Value = 1) Then WAction = PRIMOSDK_TEST Else WAction = PRIMOSDK_WRITE
    
    ' INIT THE ABORT FLAG
    bStop = False
    
    ' SET THE SPEED
    If (CurFunc <> FUNC_BUILDGI And CurFunc <> FUNC_SAVEDATA And CurFunc _
       < FUNC_AUDIOEXTRACT) Then
        Speed = cmbSpeed.ItemData(cmbSpeed.ListIndex)
    End If
    
    ' CLEAR THE FILE COUNT
    TotFileCount = 0

    '
    ' SELECT AND CALL THE APPROPRIATE FUNCTION
    '
    Select Case cmbFunction.ListIndex + 1
    Case FUNC_COPY
        AppFunc_Copy
    Case FUNC_BUILDGI
        AppFunc_BuildGI
    Case FUNC_RECORDGIORTRACK
        AppFunc_RecordGIOrTrack
    Case FUNC_VERIFYDISC
        AppFunc_VerifyDisc
    Case FUNC_VERIFYGI
        AppFunc_VerifyGI
    Case FUNC_RECORDDATA
        AppFunc_RecordData
    Case FUNC_SAVEDATA
        AppFunc_SaveData
    Case FUNC_RECORDAUDIO
        AppFunc_RecordAudio
    Case FUNC_RECORDVIDEO
        AppFunc_VideoCD
    Case FUNC_AUDIOEXTRACT
        AppFunc_AudioExtract
    Case FUNC_ERASEDISC
        AppFunc_EraseDisc
    End Select

End Sub

Private Sub cmdClearData_Click()
'
' CLEAR FILE LIST
'
    lstFileList.Clear

End Sub

Private Sub cmbFunction_Click()
'
' CHANGE THE CURRENT FUNCTION, ENABLE AND DISABLE ALL RELATED CONTROLS
'
    CurFunc = cmbFunction.ListIndex + 1
    EnableDisable CurFunc

End Sub

Private Sub cmdCloseRec_Click()
'
' CLOSE THE RECORDER'S TRAY
'
Dim Unit As Long

    Unit = cmbRecorder.ItemData(cmbRecorder.ListIndex)
    Reply = PrimoSDK_MoveMedium(Handle, Unit, PRIMOSDK_CLOSETRAY + PRIMOSDK_IMMEDIATE)
    If Reply <> PRIMOSDK_OK Then
        DisplayError Reply, "PrimoSDK_MoveMedium", ""
        Exit Sub
    End If

End Sub

Private Sub cmdCloseSource_Click()
'
' CLOSE THE SOURCE'S TRAY
'
Dim Unit As Long

    Unit = cmbSource.ItemData(cmbSource.ListIndex)
    Reply = PrimoSDK_MoveMedium(Handle, Unit, PRIMOSDK_CLOSETRAY + PRIMOSDK_IMMEDIATE)
    If Reply <> PRIMOSDK_OK Then
        DisplayError Reply, "PrimoSDK_MoveMedium", ""
        Exit Sub
    End If

End Sub

Private Sub cmdDiscInfoREC_Click()
'
' DISC INFO REC
'
Dim Unit As Long, MediumType As Long, MediumFormat As Long, Erasable As Long, Tracks As Long, Used As Long, Free As Long
Dim EraseYesNo As String
Dim SessionNumber As Long, TrackType As Long, PreGap As Long, Start As Long, Length As Long

    Unit = cmbRecorder.ItemData(cmbRecorder.ListIndex)
    Reply = PrimoSDK_DiscInfo(Handle, Unit, MediumType, MediumFormat, Erasable, Tracks, Used, Free)
    If Reply <> PRIMOSDK_OK Then
        DisplayError Reply, "PrimoSDK_DiscInfo", ""
        Exit Sub
    End If
    Select Case MediumType
        Case PRIMOSDK_SILVER
            Stri$ = "Silver or closed recordable disc"
        Case PRIMOSDK_COMPLIANTGOLD
            Stri$ = "Recordable data disc, valid for adding data tracks with PrimoSDK"
        Case PRIMOSDK_OTHERGOLD
            Stri$ = "Recordable disc, but not valid for adding data tracks with PrimoSDK"
        Case PRIMOSDK_BLANK
            Stri$ = "Blank Disc"
    End Select
    
    If Format(Erasable) = 0 Then EraseYesNo = "No" Else EraseYesNo = "Yes"
    
    Stri$ = "Type: " + Stri$ + Chr$(10) + Chr$(10) + "Format: " + Hex$(MediumFormat) + Chr$(10) + "Erasable: " + EraseYesNo + Chr$(10) + Chr$(10) + "Total Tracks: " + Format$(Tracks) + Chr$(10) + "Used sectors: " + Format$(Used) + Chr$(10) + "Free Sectors: " + Format$(Free)

    If (Tracks > 0) Then
       Stri$ = Stri$ + vbCrLf
       For i = 1 To Tracks
          Reply = PrimoSDK_TrackInfo(Handle, i, SessionNumber, TrackType, PreGap, Start, Length)
          If (Reply <> PRIMOSDK_OK) Then
            DisplayError Reply, "PrimoSDK_TrackInfo", ""
            Exit Sub
          End If
          Stri$ = Stri$ + vbCrLf + "Track " + Format(i) + " - Session: " + Format(SessionNumber) + "   Type: " + Format(TrackType) + "   PreGap: " + Format(PreGap) + "   Start: " + Format(Start) + "   Length: " + Format(Length)
       Next i
    End If

    MsgBox Stri$, 64, Caption

End Sub

Private Sub cmdDiscInfoSource_Click()
'
' DISC INFO SOURCE
'
Dim Unit As Long, MediumType As Long, MediumFormat As Long, Erasable As Long, Tracks As Long, Used As Long, Free As Long
Dim EraseYesNo As String

    Unit = cmbSource.ItemData(cmbSource.ListIndex)
    Reply = PrimoSDK_DiscInfo(Handle, Unit, MediumType, MediumFormat, Erasable, Tracks, Used, Free)
    If Reply <> PRIMOSDK_OK Then
        DisplayError Reply, "PrimoSDK_DiscInfo", ""
        Exit Sub
    End If
    Select Case MediumType
        Case PRIMOSDK_SILVER
            Stri$ = "Silver or closed recordable disc"
        Case PRIMOSDK_COMPLIANTGOLD
            Stri$ = "Recordable data disc, valid for adding data tracks with PrimoSDK"
        Case PRIMOSDK_OTHERGOLD
            Stri$ = "Recordable disc, but not valid for adding data tracks with PrimoSDK"
        Case PRIMOSDK_BLANK
            Stri$ = "Blank Disc"
    End Select
    
    If Format(Erasable) = 0 Then EraseYesNo = "No" Else EraseYesNo = "Yes"
    
    Stri$ = "Type: " + Stri$ + Chr$(10) + Chr$(10) + "Format: " + Hex$(MediumFormat) + Chr$(10) + "Erasable: " + EraseYesNo + Chr$(10) + Chr$(10) + "Total Tracks: " + Format$(Tracks) + Chr$(10) + "Used sectors: " + Format$(Used) + Chr$(10) + "Free Sectors: " + Format$(Free)

    If (Tracks > 0) Then
       Stri$ = Stri$ + vbCrLf
       For i = 1 To Tracks
          Reply = PrimoSDK_TrackInfo(Handle, i, SessionNumber, TrackType, PreGap, Start, Length)
          If (Reply <> PRIMOSDK_OK) Then
            DisplayError Reply, "PrimoSDK_TrackInfo", ""
            Exit Sub
          End If
          Stri$ = Stri$ + vbCrLf + "Track " + Format(i) + " - Session: " + Format(SessionNumber) + "   Type: " + Format(TrackType) + "   PreGap: " + Format(PreGap) + "   Start: " + Format(Start) + "   Length: " + Format(Length)
       Next i
    End If

    MsgBox Stri$, 64, Caption

End Sub

Private Sub cmddriveInfoREC_Click()
'
' UNIT INFO
'
Dim Unit As Long
Dim UnitType As Long
Dim UnitReady As Long
Dim Descr As String * 64
    
    Unit = cmbRecorder.ItemData(cmbRecorder.ListIndex)
    Reply = PrimoSDK_UnitInfo(Handle, Unit, UnitType, Descr$, UnitReady)
    If Reply <> PRIMOSDK_OK Then
        DisplayError Reply, "PrimoSDK_UnitInfo", ""
    End If
    
    Select Case UnitType
        Case PRIMOSDK_CDROM
            Stri$ = "CD-ROM"
        Case PRIMOSDK_CDR
            Stri$ = "CD-R"
        Case PRIMOSDK_CDRW
            Stri$ = "CD-RW"
        Case PRIMOSDK_DVDROM
            Stri$ = "DVD-ROM"
        Case PRIMOSDK_DVDR
            Stri$ = "DVD-R"
        Case PRIMOSDK_DVDRW
            Stri$ = "DVD-RW"
        Case PRIMOSDK_DVDPRW
            Stri$ = "DVD+RW"
        Case PRIMOSDK_DVDRAM
            Stri$ = "DVD-RAM"
        Case PRIMOSDK_OTHER
            Stri$ = "Not managed type"
    End Select
    
    If (UnitReady = 1) Then
       Stri2$ = "Ready"
    Else
       Stri2$ = "Not ready"
    End If
    
    Host = Unit \ 16777216
    Target = (Unit \ 65536) And 255
    LUN = (Unit \ 256) And 255
    DriveLetter = Left(cmbRecorder.Text, 1)
    MsgBox "Type: " + Stri$ & Chr(10) & Chr(10) & "Host:" & Host & "  ID:" & Target & "  LUN:" & LUN & "   Assigned drive letter: " & "'" & DriveLetter & "'" & Chr(10) & Chr(10) & "Drive: " + VBStr(Descr$) + Chr$(10) + Chr$(10) + "Status: " + Stri2$, vbInformation

End Sub

Private Sub cmdDriveInfoSource_Click()
'
' UNIT INFO
'
Dim Unit As Long
Dim UnitType As Long
Dim UnitReady As Long
Dim Descr As String * 64
    
    Unit = cmbSource.ItemData(cmbSource.ListIndex)
    Reply = PrimoSDK_UnitInfo(Handle, Unit, UnitType, Descr$, UnitReady)
    If (Reply <> 0) Then
        DisplayError Reply, "PrimoSDK_UnitInfo", ""
        Exit Sub
    End If
    
    Select Case UnitType
        Case PRIMOSDK_CDROM
            Stri$ = "CD-ROM"
        Case PRIMOSDK_CDR
            Stri$ = "CD-R"
        Case PRIMOSDK_CDRW
            Stri$ = "CD-RW"
        Case PRIMOSDK_DVDROM
            Stri$ = "DVD-ROM"
        Case PRIMOSDK_DVDR
            Stri$ = "DVD-R"
        Case PRIMOSDK_DVDRW
            Stri$ = "DVD-RW"
        Case PRIMOSDK_DVDRAM
            Stri$ = "DVD-RAM"
        Case PRIMOSDK_OTHER
            Stri$ = "Not managed type"
    End Select
    
    If (UnitReady = 1) Then
       Stri2$ = "Ready"
    Else
       Stri2$ = "Not ready"
    End If
    
    Host = Unit \ 16777216
    Target = (Unit \ 65536) And 255
    LUN = (Unit \ 256) And 255
    DriveLetter = Left(cmbSource.Text, 1)
    MsgBox "Type: " + Stri$ & Chr(10) & Chr(10) & "Host:" & Host & "  ID:" & Target & "  LUN:" & LUN & "   Assigned drive letter: " & "'" & DriveLetter & "'" & Chr(10) & Chr(10) & "Drive: " + VBStr(Descr$) + Chr$(10) + Chr$(10) + "Status: " + Stri2$, vbInformation

End Sub

Private Sub cmdEjectREC_Click()
'
' OPEN TRAY
'
Dim Unit As Long

    Unit = cmbRecorder.ItemData(cmbRecorder.ListIndex)
    Reply = PrimoSDK_MoveMedium(Handle, Unit, PRIMOSDK_OPENTRAYEJECT + PRIMOSDK_IMMEDIATE)
    If Reply <> PRIMOSDK_OK Then
        DisplayError Reply, "PrimoSDK_MoveMedium", ""
        Exit Sub
    End If

End Sub

Private Sub cmdEjectSource_Click()
'
' OPEN TRAY
'
Dim Unit As Long

    Unit = cmbSource.ItemData(cmbSource.ListIndex)
    Reply = PrimoSDK_MoveMedium(Handle, Unit, PRIMOSDK_OPENTRAYEJECT + PRIMOSDK_IMMEDIATE)
    If Reply <> PRIMOSDK_OK Then
        DisplayError Reply, "PrimoSDK_MoveMedium", ""
        Exit Sub
    End If

End Sub

Private Sub lstFileList_OLEDragOver(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single, State As Integer)
'
'SET THE EFFECT FOR THE DRAG AND DROP
'
    Effect = vbDropEffectCopy And Effect

End Sub

Private Sub lstFileList_OLEDragDrop(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single)
'
' PROCESS THE DRAG AND DROP
'
Dim FileToAdd As String
Dim fname As Variant
Dim Extension As String

    For Each fname In Data.Files
        FileToAdd = fname
        'CHECK IF IT IS AN AUDIO JOB
        If CurFunc = 8 Then
            Extension = Right(FileToAdd, 4)
            If (UCase(Extension) = ".WAV") Then
                'ADD THE FILE TO THE LIST
                lstFileList.AddItem FileToAdd
                FileToAdd = ""
                MainForm.Refresh
            End If
        ElseIf CurFunc = 9 Then
            Extension = Right(FileToAdd, 4)
            If (UCase(Extension) = ".DAT") Or (UCase(Extension) = ".MPEG") Or _
                (UCase(Extension) = ".MPG") Then
                'ADD THE FILE TO THE LIST
                lstFileList.AddItem FileToAdd
                FileToAdd = ""
                MainForm.Refresh
            End If
        Else
            'ADD ALL THE FILES IF DATA DISC
            lstFileList.AddItem FileToAdd
            FileToAdd = ""
            MainForm.Refresh
        End If
    Next fname
    
    MainForm.Refresh

End Sub

Function EnableDisable(CurFunc)
'       ===============
'
' ENABLE AND DISABLE CONTROLS ACCORDING TO THE FUNCTION SELECTED
'
'
Dim bSel As Boolean
Dim bShow As Boolean

    ' OPERATION SECTION
    cmbFunction.Enabled = Not bBusy
    bSel = CurFunc <> FUNC_BUILDGI And CurFunc <> FUNC_ERASEDISC And CurFunc <> FUNC_VERIFYDISC And _
           CurFunc <> FUNC_VERIFYGI And CurFunc <> FUNC_AUDIOEXTRACT And CurFunc <> FUNC_SAVEDATA And (Not bBusy)
    chkTest.Enabled = bSel
    bSel = CurFunc <> FUNC_BUILDGI And CurFunf <> FUNC_VERIFYGI And CurFunc <> FUNC_SAVEDATA And _
           CurFunc <> FUNC_AUDIOEXTRACT And CurFunc <> FUNC_ERASEDISC And (Not bBusy)
    cmbSpeed.Enabled = bSel
    chkRecToAll.Enabled = bSel And (HowManyRecorder > 0)
    If (bBusy) Then btnGo.Caption = "St&op" Else btnGo.Caption = "G&o !"
    ' THE ERASE DISC CAN'T BE ABORTED
    btnGo.Enabled = Not (bBusy And CurFunc = FUNC_ERASEDISC)
    If (bBusy) Then txtResult.Text = "Working..." Else txtResult.Text = ""
    lstFileList.Enabled = (CurFunc = FUNC_RECORDDATA Or CurFunc = FUNC_SAVEDATA Or _
                           CurFunc = FUNC_RECORDAUDIO Or CurFunc = FUNC_RECORDVIDEO) And Not bBusy
    ' RECORDER SECTION
    bSel = CurFunc <> FUNC_BUILDGI And CurFunc <> FUNC_SAVEDATA And CurFunc <> FUNC_AUDIOEXTRACT And _
           (Not bBusy) And (HowManyRecorder > 0)
    cmbRecorder.Enabled = bSel And (HowManyRecorder > 0)
    cmdEjectRec.Enabled = bSel
    cmdCloseRec.Enabled = bSel
    cmddriveInfoRec.Enabled = bSel
    cmdDiscInfoRec.Enabled = bSel
    ' SOURCE DRIVE SECTION
    bSel = (CurFunc < FUNC_RECORDGIORTRACK Or CurFunc = FUNC_VERIFYDISC Or _
           CurFunc = FUNC_AUDIOEXTRACT) And (Not bBusy)
    lblSource.Enabled = bSel
    cmbSource.Enabled = bSel
    cmdEjectSource.Enabled = bSel
    cmdCloseSource.Enabled = bSel
    cmdDiscInfoSource.Enabled = bSel
    cmdDriveInfoSource.Enabled = bSel
    bShow = (CurFunc < FUNC_RECORDGIORTRACK Or CurFunc = FUNC_VERIFYDISC Or _
            CurFunc = FUNC_AUDIOEXTRACT)
    cmbSource.Visible = bShow
    cmdEjectSource.Visible = bShow
    cmdCloseSource.Visible = bShow
    cmdDiscInfoSource.Visible = bShow
    cmdDriveInfoSource.Visible = bShow
    lblSource.Visible = bShow
    ' AUDIO EXTRACT TRACK
    bSel = (CurFunc = FUNC_AUDIOEXTRACT) And Not bBusy
    lblTrackExtract.Enabled = bSel
    txtEditTrack.Enabled = bSel
    bShow = (CurFunc = FUNC_AUDIOEXTRACT)
    txtEditTrack.Visible = bShow
    lblTrackExtract.Visible = bShow
    ' FILE LIST SECTION
    bSel = (CurFunc = FUNC_RECORDDATA Or CurFunc = FUNC_SAVEDATA Or CurFunc = FUNC_RECORDAUDIO Or _
            CurFunc = FUNC_RECORDVIDEO) And Not bBusy
    cmdClearData.Enabled = bSel
    lstFileList.Enabled = bSel
    lblFileListString.Enabled = bSel
    bShow = (CurFunc = FUNC_RECORDDATA Or CurFunc = FUNC_SAVEDATA Or CurFunc = FUNC_RECORDAUDIO Or _
            CurFunc = FUNC_RECORDVIDEO)
    cmdClearData.Visible = bShow
    lstFileList.Visible = bShow
    lblFileListString.Visible = bShow
    ' DATA DISC PARAMETERS SECTION
    bSel = (CurFunc = FUNC_RECORDDATA Or CurFunc = FUNC_SAVEDATA) And Not bBusy
    '
    lblVolume.Enabled = bSel
    txtVOLUMENAME.Enabled = bSel
    optIsoLevel.Enabled = bSel
    optJoliet.Enabled = bSel
    optUDF.Enabled = bSel
    bShow = (CurFunc = FUNC_RECORDDATA Or CurFunc = FUNC_SAVEDATA)
    lblVolume.Visible = bShow
    txtVOLUMENAME.Visible = bShow
    optIsoLevel.Visible = bShow
    optJoliet.Visible = bShow
    optUDF.Visible = bShow
    '
    bSel = CurFunc = FUNC_RECORDDATA And Not bBusy
    optMode1.Enabled = bSel
    optMode2.Enabled = bSel
    lblswap.Enabled = bSel
    sliSwap.Enabled = bSel
    lblLoad.Enabled = bSel
    lblSwapResult.Enabled = bSel
    txtload.Enabled = bSel
    bShow = (CurFunc = FUNC_RECORDDATA)
    optMode1.Visible = bShow
    optMode2.Visible = bShow
    lblswap.Visible = bShow
    sliSwap.Visible = bShow
    lblLoad.Visible = bShow
    lblSwapResult.Visible = bShow
    txtload.Visible = bShow
    '
    bSel = (CurFunc = FUNC_RECORDDATA Or CurFunc = FUNC_RECORDAUDIO) And Not bBusy
    chkCloseDisc.Enabled = bSel
    bShow = (CurFunc = FUNC_RECORDDATA Or CurFunc = FUNC_RECORDAUDIO)
    chkCloseDisc.Visible = bShow

End Function

Private Sub Picture1_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)

    ' SHOW SUPPORTED UNITS
    If (Not bBusy) Then
       Stri$ = String(50000, Chr(0))
       Tot = PrimoSDK_ListSupportedUnits(Stri$)
       VBStr Stri$
       txtResult.Text = Format(Tot) + " supported recorders:" + vbCrLf + vbCrLf + Stri$
    End If
    
End Sub

Private Sub ProgTimer_Timer()
'          =================
'
' TIMER EVENT, ALL THE HANDLING WHILE RUNNING HAPPENS HERE
'
Dim UnitType As Long
Dim UnitReady As Long
Dim dwRepStat As String
Dim Stri$
Dim dwSize As Long
Dim dwTotal As Long
Dim SelectedFunction As String
'ERROR CODES
Dim pdwCommand As Long
Dim pdwSense As Long
Dim pdwASC As Long
Dim pdwASCQ As Long
Dim Mesg As String

    ' ON EACH TIMER ENTRY
    lstFileList.ListIndex = -1
    PicExtract.Refresh
    
    ' SET THE ABORT FLAG IF NEEDED
    If (bStop) Then Flg = PRIMOSDK_ABORT Else Flg = PRIMOSDK_STATUS
    
    ' ASK THE OPERATION STATUS
    dwRepStat = PrimoSDK_RunningStatus(Handle, Flg, dwSize, dwTotal)
    
    '
    'DISPLAY RUNNING STATUS IN PROGRESS BAR HERE
    '
    SelectedFunction = "SelFunc_" & CurFunc

    Select Case CurFunc
    Case 1
        GoTo Default
    Case 2
        'BUILD GI SELECTED
        txtResult.Text = ("Build Global-Image - Sector " & dwSize & " of " & dwTotal) & vbCrLf
        GoTo All
    Case 3
        GoTo Default
    Case 4, 5
        'VERIFY DISC AND VERYFY GI
        txtResult.Text = ("Verifying - Sector " & dwSize & " of " & dwTotal) & vbCrLf
        '
        'LOOP FOR EVERY DRIVE ENGAGED IN THE OPERATION AND GET ITS STATUS
        '
        txtResult.Text = ("Verifying - Sector " & dwSize & " of " & dwTotal) & vbCrLf

        For i = LBound(UnitsRec) To UBound(UnitsRec)
    
        If (UnitsRec(i) = -1) Then Exit For
            Reply = PrimoSDK_UnitStatus(Handle, UnitsRec(i), pdwCommand, _
                                        pdwSense, pdwASC, pdwASCQ)
            Host = UnitsRec(i) \ 16777216
            Target = (UnitsRec(i) \ 65536) And 255
            LUN = (UnitsRec(i) \ 256) And 255
            DriveLetter = (UnitsRec(i) And 255)
            DriveLetter = Chr(DriveLetter)
            If Reply = PRIMOSDK_OK Then
                txtResult.Text = txtResult.Text & "   " & DriveLetter & ": " & "(h" & Format$(Host) & " id" & Format$(Target) & ") - OK" & vbCrLf
            Else
            IO$ = txtResult.Text & "   " & DriveLetter & ": " & "(h" & Format$(Host) & " id" & Format$(Target) & ") - Err:" & Reply & " -"
            txtResult.Text = IO$ & "Verify Error"
            End If
        Next i
        GoTo All
    Case 6
        GoTo Default
    Case 7
        'SAVE DATA SELECTED
        txtResult.Text = ("SaveData Image - Sector " & dwSize & " of " & dwTotal) & vbCrLf
        GoTo All
    Case 8
        GoTo Default
    Case 9
        GoTo Default
    Case 10
        'AUDIO EXTRACT SELECTED
        txtResult.Text = ("Audio Extraction - Sector " & dwSize & " of " & dwTotal) & vbCrLf
        GoTo All
    Case FUNC_ERASEDISC
        GoTo Default
    End Select
    
    Exit Sub
         
Default:
    '
    ' LOOP FOR EVERY DRIVE ENGAGED IN THE OPERATION, GET ITS STATUS
    '
    If (WAction = FUNC_ERASEDISC) Then
        txtResult.Text = "Erasing" & vbCrLf
    Else
        If (WAction = PRIMOSDK_TEST) Then
            Mesg = "Testing - Sector "
        ElseIf (WAction = PRIMOSDK_WRITE) Then
            Mesg = "Writing - Sector "
        End If
        txtResult.Text = (Mesg & dwSize & " of " & dwTotal) & vbCrLf
    End If
    
    For i = LBound(UnitsRec) To UBound(UnitsRec)
    
        If (UnitsRec(i) = -1) Then Exit For
        Reply = PrimoSDK_UnitStatus(Handle, UnitsRec(i), pdwCommand, _
                                    pdwSense, pdwASC, pdwASCQ)
        Host = UnitsRec(i) \ 16777216
        Target = (UnitsRec(i) \ 65536) And 255
        LUN = (UnitsRec(i) \ 256) And 255
        DriveLetter = (UnitsRec(i) And 255)
        DriveLetter = Chr(DriveLetter)
        If Reply = PRIMOSDK_OK Then
            txtResult.Text = txtResult.Text & "   " & DriveLetter & ": " & "(h" & Format$(Host) & " id" & Format$(Target) & ") - OK" & vbCrLf
        Else
            IO$ = txtResult.Text & "   " & DriveLetter & ": " & "(h" & Format$(Host) & " id" & Format$(Target) & ") - Err:" & Reply & " -"
            txtResult.Text = IO$ & (" Cmd: " & Hex(pdwCommand) & " Sense:" & Hex(pdwSense) & _
                             " ASC: " & Hex(pdwASC) & " ASCQ: " & Hex(pdwASCQ)) & vbCrLf
        End If
    Next i

All:
    '
    ' SHOW THE PROGRESS STATUS IN THE STATUS BAR
    '
    If CurFunc <> FUNC_ERASEDISC And dwTotal <> 0 Then
        prbProgress.Max = dwTotal
        prbProgress.Value = dwSize
    End If
    
    '
    ' IF WE ARE STILL RUNNING JUST CONTINUE THE TIMER
    '
    If dwRepStat = PRIMOSDK_RUNNING Then Exit Sub
    
    '
    ' FOR GOOD OR FOR BAD WE FINISHED, STOP THE TIMER
    '
    ProgTimer.Enabled = False
    
    '
    ' HOUSEKEEPING
    '
    If CurFunc = FUNC_RECORDDATA Or CurFunc = FUNC_SAVEDATA Then PrimoSDK_CloseImage (Handle)
    If CurFunc = FUNC_RECORDAUDIO Then PrimoSDK_CloseAudio (Handle)
    If CurFunc = FUNC_RECORDVIDEO Then PrimoSDK_CloseVideoCD (Handle)
    If CurFunc = FUNC_BUILDGI Or CurFunc = FUNC_AUDIOEXTRACT Then LockAndBlockSource UnitSource, False
    If CurFunc <> FUNC_BUILDGI And CurFunc <> FUNC_SAVEDATA And CurFunc <> FUNC_AUDIOEXTRACT Then
        'LOOP AND EJECT
        LockAndBlock UnitsRec, False
        For i = LBound(UnitsRec) To UBound(UnitsRec)
            '
            If (UnitsRec(i) = -1) Then Exit For
            Reply = PrimoSDK_MoveMedium(Handle, UnitsRec(i), PRIMOSDK_OPENTRAYEJECT + PRIMOSDK_IMMEDIATE)
        Next i
    End If
        
    '
    ' NOW SEE WHAT HAPPENED AND PUT APPROPRIATE FINAL MESSAGE
    '
    If (dwRepStat = PRIMOSDK_OK) Then
    
        Select Case CurFunc
        
        Case FUNC_BUILDGI
            Resu$ = ("The Global-Image has been successfully built.")
        Case FUNC_SAVEDATA
            Resu$ = ("The build of the image successfully completed.")
        Case FUNC_VERIFYDISC, FUNC_VERIFYGI
            Resu$ = ("Verify completed.")
        Case FUNC_AUDIOEXTRACT
            Resu$ = ("Audio Extraction successfully completed.")
        Case FUNC_ERASEDISC
             Resu$ = ("Erase completed.")
        Case Else
            If (WAction = PRIMOSDK_TEST) Then
                Resu$ = "Test Completed."
            ElseIf (WAction = PRIMOSDK_WRITE) Then Resu$ = "Write Completed."
            End If
        End Select
        'FINSIHED OK
        MsgBox Resu$, vbInformation
    ElseIf (dwRepStat <> PRIMOSDK_OK) Then
        'FINISHED WITH ERRORS
         DisplayError dwRepStat, "PrimoSDK_RunningStatus", ""
    End If
        
    '
    ' CLEAR THE PROGRESS AND RETURN TO NORMAL
    '
    prbProgress.Value = 0
    txtResult.Text = ""
    bBusy = False
    EnableDisable (CurFunc)
    txtResult.Text = ""
    MainForm.Refresh
       
End Sub

Function AppFunc_Copy()
'       ==============
'
' COPY DISC
'
'
Dim Unit As Long
Dim MediumType As Long
Dim MediumFormat As Long
Dim Erasable As Long
Dim Tracks As Long
Dim Used As Long
Dim Free As Long
Dim Rec
Dim dwErr
    
    '
    ' PUT IN LetterSource THE DRIVE LETTER TO RECORD FORM AND CHECK THE CONTENTS
    '
    UnitSource = cmbSource.ItemData(cmbSource.ListIndex)
    dwErr = PrimoSDK_DiscInfo(Handle, UnitSource, MediumType, MediumFormat, Erasable, Tracks, Used, Free)
    If (dwErr <> PRIMOSDK_OK) Then
        DisplayError dwErr, "PrimoSDK_DiscInfo", ""
        Exit Function
    End If
    If (MediumType = PRIMOSDK_BLANK) Then
        MsgBox "The source disc is blank.", vbInformation
        Exit Function
    End If
    '
    'SET THE RECORDING STATUS
    '
    bBusy = True
    EnableDisable CurFunc
    'DISPLAY THE NEEDED SECTORS AND WAIT A WHILE
    txtResult.Text = (Used & " sectors to copy...") & vbCrLf
    txtResult.Refresh
    Sleep (1000)
    'COPY OR TEST
    txtResult.Text = ("Starting the copy...") & vbCrLf
    txtResult.Refresh
    '
    If (Not (LockAndBlockSource(UnitSource, True))) Then
        bBusy = False
        EnableDisable CurFunc
    End If
    
    If (Not (LockAndBlock(UnitsRec, True))) Then
        bBusy = False
        EnableDisable CurFunc
    End If
    '
    dwErr = PrimoSDK_CopyDisc(Handle, UnitsRec(1), UnitSource, WAction, Speed)
    If (dwErr <> PRIMOSDK_OK) Then
        'AN ERROR AS HAPPENED IMMEDIATELY DISPLAY IT
        DisplayError dwErr, "PrimoSDK_CopyDisc", ""
        LockAndBlockSource UnitSource, False
        LockAndBlock UnitsRec, False
        bBusy = False
        EnableDisable CurFunc
    Else
        'THES COPY STARTED OK, LET THE SYSTEM GO CONTROLLED BY THE TIMER
        ProgTimer.Enabled = True
    End If
    
End Function

Function AppFunc_BuildGI()
'       =================
'
' BUILD GI IMAGE
'
'
Dim szImageName As String

    '
    'ASK THE GI NAME
    '
    cdaDialog.DialogTitle = "Global Image to Build"
    cdaDialog.Filter = "Global Image (*.gi)|*.gi|"
    cdaDialog.Flags = 2 + 4
    cdaDialog.ShowSave
    MainForm.Refresh
    If cdaDialog.Filename = "" Then Exit Function
    MainForm.Refresh
    If Right(UCase(cdaDialog.Filename), 3) <> ".GI" Then
        szImageName = cdaDialog.Filename & ".gi"
    Else
        szImageName = cdaDialog.Filename
    End If
    
    bBusy = True
    EnableDisable CurFunc
    If Not (LockAndBlockSource(UnitSource, True)) Then
        bBusy = False
        EnableDisable CurFunc
        Exit Function
    End If
    '
    'BUILD THE IMAGE
    '
    dwErr = PrimoSDK_ReadGI(Handle, UnitSource, szImageName, PRIMOSDK_COPYPREGAP)
    If (dwErr <> PRIMOSDK_OK) Then
        ' AN ERROR HAPPENED CREATING THE TEMPORARY
        DisplayError dwErr, "PrimoSDK_ReadGI", ""
        ' DELETE THE FILE, JUST IN CASE IT WAS STILL THERE
        Kill szImageName
        bBusy = False
        EnableDisable CurFunc
    Else
        ProgTimer.Enabled = True
    End If

End Function

Function AppFunc_RecordGIOrTrack()
'       =========================
'
' RECORD GI IMAGE OR OTHER IMAGE
'
'
Dim szImageName As String

    '
    'ASK THE GI NAME
    '
    cdaDialog.DialogTitle = "Global Image or Other Image to Record"
    cdaDialog.Filter = "Global Image (*.gi)|*.gi|Iso Image (*.iso)|*.iso|UDF Image (*.udi)|*.udi|"
    cdaDialog.Flags = 4
    MainForm.Refresh
    cdaDialog.ShowOpen
    If cdaDialog.Filename = "" Then Exit Function
    szImageName = cdaDialog.Filename
    bBusy = True
    EnableDisable CurFunc
    If Not (LockAndBlock(UnitsRec, True)) Then
        bBusy = False
        EnableDisable CurFunc
        Exit Function
    End If
    'IF "ISO" IT IS AN ISO IMAGE (ANOTHER IMAGE), OTHERWISE GI
    If UCase(Right(szImageName, 3)) = UCase(".GI") Then
        dwErr = PrimoSDK_WriteGI(Handle, UnitsRec(1), szImageName, WAction, Speed)
        If (dwErr <> PRIMOSDK_OK) Then
            DisplayError dwErr, "PrimoSDK_WriteGI", ""
            bBusy = False
            EnableDisable CurFunc
            LockAndBlock UnitsRec, False
        End If
    Else
        WAction2 = WAction + PRIMOSDK_CLOSEDISC + PRIMOSDK_IMAGE_M1_48
        dwErr = PrimoSDK_WriteOtherCDImage(Handle, UnitsRec(1), szImageName, WAction2, Speed)
        If (dwErr <> PRIMOSDK_OK) Then
            DisplayError dwErr, "PrimoSDK_WriteOtherCDImage", ""
            bBusy = False
            EnableDisable CurFunc
            LockAndBlock UnitsRec, False
        End If
    End If
    
    'WRITING STARTED, START THE TIMER
    ProgTimer.Enabled = True
    
End Function

Function AppFunc_VerifyDisc()
'       ====================
'
' VERIFY DISC
'
'
Dim Unit As Long
Dim MediumType As Long
Dim MediumFormat As Long
Dim Erasable As Long
Dim Tracks As Long
Dim Used As Long
Dim Free As Long
Dim Rec
Dim dwErr
    
    '
    'PUT IN LetterSource THE DRIVE LETTER TO RECORD FORM AND CHECK THE CONTENTS
    '
    UnitSource = cmbSource.ItemData(cmbSource.ListIndex)
    dwErr = PrimoSDK_DiscInfo(Handle, UnitSource, MediumType, MediumFormat, Erasable, Tracks, Used, Free)
    If (dwErr <> PRIMOSDK_OK) Then
        DisplayError dwErr, "PrimoSDK_DiscInfo", ""
        Exit Function
    End If
    If (MediumType = PRIMOSDK_BLANK) Then
        MsgBox "The source disc is blank.", vbInformation
        Exit Function
    End If
    ' SET THE RECORDING STATUS
    bBusy = True
    EnableDisable CurFunc
    ' VERIFY
    txtResult.Text = ("Starting the verify...") & vbCrLf
    '
    If Not (LockAndBlock(UnitsRec, True)) Then
        bBusy = False
        EnableDisable CurFunc
        Exit Function
    End If
    dwErr = PrimoSDK_VerifyDisc(Handle, UnitsRec(1), UnitSource, Speed)
    If (dwErr <> PRIMOSDK_OK) Then
        'AN ERROR HAPPENED DISPLAY IT NOW
        DisplayError dwErr, "PrimoSDK_VerifyDisc", ""
        bBusy = False
        EnableDisable CurFunc
        LockAndBlock UnitsRec, False
    Else
        'VERIFY STARTED CONTUNUE WITH THE TIMER
        ProgTimer.Enabled = True
    End If
        
End Function

Function AppFunc_VerifyGI()
'       ==================
'
' VERIFY GI IMAGE
'
'
Dim szImageName As String

    '
    ' ASK THE GI NAME
    '
    cdaDialog.Filter = "Global Image (*.gi)|*.gi|Iso Image (*.iso)|*.iso|UDF Image (*.udi)|*.udi|"
    cdaDialog.Flags = 4
    cdaDialog.DialogTitle = "Global Image"
    MainForm.Refresh
    cdaDialog.ShowOpen
    MainForm.Refresh
    If cdaDialog.Filename = "" Then Exit Function
    szImageName = cdaDialog.Filename
    bBusy = True
    EnableDisable CurFunc
    If Not (LockAndBlock(UnitsRec, True)) Then
        bBusy = False
        EnableDisable CurFunc
        Exit Function
    End If
    ' VERIFY
    txtResult.Text = ("Starting the verify...") & vbCrLf
    '
    If Not (LockAndBlock(UnitsRec, True)) Then
        bBusy = False
        EnableDisable CurFunc
        Exit Function
    End If
    dwErr = PrimoSDK_VerifyGI(Handle, UnitsRec(1), szImageName, Speed)
    If (dwErr <> PRIMOSDK_OK) Then
        'AN ERROR HAPPENED DISPLAY IT NOW
        DisplayError dwErr, "PrimoSDK_VerifyGI", ""
        bBusy = False
        EnableDisable CurFunc
        LockAndBlock UnitsRec, False
    Else
        'VERIFY STARTED CONTUNUE WITH THE TIMER
        ProgTimer.Enabled = True
    End If
    
End Function

Function AppFunc_RecordData()
'       ====================
'
' RECORD DATA DISC
'
'
Dim dwFSMode As Long
Dim dwCDMode As Long
Dim dwDateMode As Long
Dim dwCloseDisc As Long
Dim TotEntries As Long
Dim dwFileSwap As Long
Dim Total As Long

    ' MATCH SLIDER WITH VARIABLE
    dwFileSwap = sliSwap.Value
    
    ' CHECK THAT THE FILE LIST IS NOT EMPTY
    TotEntries = lstFileList.ListCount
    If TotEntries = 0 Then
        MsgBox "The recording list is empty. Drag some file in it and try again.", vbInformation
        Exit Function
    End If
    
    ' SET THE RECORDING STATUS
    bBusy = True
    EnableDisable CurFunc
    Me.Refresh
    
    ' PREPARE THE PARAMETERS FOR THE NEW IMAGE
    If optJoliet.Value = True Then dwFSMode = PRIMOSDK_JOLIET
    If optMode1.Value = True Then dwCDMode = PRIMOSDK_MODE1 Else dwCDMode = PRIMOSDK_MODE2
    If optUDF.Value = True Then
        dwFSMode = PRIMOSDK_UDF
        dwCDMode = PRIMOSDK_MODE1
    ElseIf optIsoLevel.Value = True Then
        dwFSMode = PRIMOSDK_ISOLEVEL1
        If optMode1.Value = True Then dwCDMode = PRIMOSDK_MODE1 Else dwCDMode = PRIMOSDK_MODE2
    End If
    ' SET THE DATE AND TRACK TO LOAD
    dwDateMode = PRIMOSDK_ORIGDATE
    If chkCloseDisc.Value = 1 Then dwCloseDisc = PRIMOSDK_CLOSEDISC Else dwCloseDisc = 0
    '
    dwLoad = Val(txtload.Text)
    If (dwLoad <> 0) Then
        txtResult.Text = ("Loading track: " & dwLoad) & vbCrLf
    End If
    txtResult.Refresh
    
    'MAKE THE NEW IMAGE FOR THE DATA DISC
    If dwFileSwap = 257 Then dwFileSwap = -1
    Flags = dwFSMode + dwCDMode + dwDateMode + dwCloseDisc + PRIMOSDK_CHECKDUPLI
    ' SET THE TEMP FOLDER TO ROOT OF C:
    TempFolder = "C:\"
    
    dwErr = PrimoSDK_NewImage(Handle, UnitsRec(1), txtVOLUMENAME.Text, dwLoad, Flags, dwFileSwap, TempFolder)
    If (dwErr <> PRIMOSDK_OK) Then
        DisplayError dwErr, "PrimoSDK_NewImage", ""
        bBusy = False
        EnableDisable CurFunc
        LockAndBlock UnitsRec, False
        Exit Function
    End If
    '
    ' LOAD THE FILES IN THE IMAGE
    '
    For i = 0 To TotEntries - 1
        lstFileList.ListIndex = i
        
        DirOrFile = GetAttr(lstFileList.Text) And vbDirectory
        If (DirOrFile = vbDirectory) Then
            'ITS A FOLDER ADD THE ROOTS FOLDERS AND THE CONTENTS
            CutAndAddFolders lstFileList.Text, False
            FileAdder (lstFileList.Text & "\")
        ElseIf (DirOrFile <> vbDirectory) Then
            'ITS A FILE
            CutAndAddFolders lstFileList.Text, True
        End If
    Next i

    If (Not LockAndBlock(UnitsRec, True)) Then
        PrimoSDK_CloseImage (Handle)
        bBusy = False
        EnableDisable (CurFunc)
    End If
    
    '
    'WRITE OR TEST
    '
    dwErr = PrimoSDK_WriteImage(Handle, WAction, Speed, Total)
    If (dwErr <> PRIMOSDK_OK) Then
        ' AN ERROR HAPPENED
        DisplayError dwret, "PrimoSDK_WriteImage", ""
        PrimoSDK_CloseImage (Handle)
        bBusy = False
        EnableDisable (CurFunc)
        LockAndBlock UnitsRec, False
    Else
        txtResult.Text = Total & " secors are needed for these " & TotFileCount & " files"
        txtResult.Refresh
        Sleep (1000)
        ' WRITING STARTED LET THE SYSTEM GOP CONTROLLED BY THE TIMER
        ProgTimer.Enabled = True
    End If

End Function

Function AppFunc_SaveData()
'       ==================
'
' SAVE DATA DISC
'
'
Dim TotEntries As Long
Dim dwCloseDisc As Long
Dim dwSize As Long
Dim FileNameToSave As String
Dim strFileName As String
Dim Total As Long

    TotEntries = lstFileList.ListCount
    'CHECK THAT THE FILE LIST IS NOT EMPTY
    If TotEntries = 0 Then
        MsgBox "The recording list is empty. Drag some file in it and try again.", vbInformation
        Exit Function
    End If
    '
    ' ASK THE FILE NAME
    '
    cdaDialog.DialogTitle = "File Where to Write the Image"
    cdaDialog.Filter = "Iso Track Image (*.iso)|*.iso|UDF Track Image (*.udi)|*.udi|"
    cdaDialog.Flags = 4 + 2
    cdaDialog.ShowSave
    If cdaDialog.Filename = "" Then Exit Function
    strFileName = cdaDialog.Filename
    ' SET THE RECORDING STATUS
    bBusy = True
    EnableDisable CurFunc
    ' PREPARE THE PARAMETERS FOR THE NEW IMAGE
    If optJoliet.Value = True Then dwFSMode = PRIMOSDK_JOLIET
    If optMode1.Value = True Then dwCDMode = PRIMOSDK_MODE1 Else dwCDMode = PRIMOSDK_MODE2
    If optUDF.Value = True Then
        dwFSMode = PRIMOSDK_UDF
        dwCDMode = PRIMOSDK_MODE1
    ElseIf optIsoLevel.Value = True Then
        dwFSMode = PRIMOSDK_ISOLEVEL1
        If optMode1.Value = True Then dwCDMode = PRIMOSDK_MODE1 Else dwCDMode = PRIMOSDK_MODE2
    End If
    '
    ' MAKE THE NEW IMAGE FOR THE DATA DISK
    '
    UnitsRec(1) = &HFFFFFFFF
    ' SET THE DATE
    dwDateMode = PRIMOSDK_ORIGDATE
    If chkCloseDisc.Value = 1 Then dwCloseDisc = PRIMOSDK_CLOSEDISC Else dwCloseDisc = 0
    dwFileSwap = -1
    Flags = dwFSMode + dwCDMode + dwDateMode + dwCloseDisc + PRIMOSDK_CHECKDUPLI
    ' SET THE TEMP FOLDER TO ROOT OF C:
    TempFolder = "C:\"
    
    dwErr = PrimoSDK_NewImage(Handle, UnitsRec(1), txtVOLUMENAME.Text, 0, Flags, dwFileSwap, TempFolder)
    '
    If (dwErr <> PRIMOSDK_OK) Then
        DisplayError dwErr, "PrimoSDK_NewImage", ""
        bBusy = False
        EnableDisable CurFunc
        LockAndBlock UnitsRec, False
        Exit Function
    End If
    '
    ' LOAD THE FILES IN THE IMAGE
    '
    For i = 0 To TotEntries - 1
        lstFileList.ListIndex = i
        
        DirOrFile = GetAttr(lstFileList.Text) And vbDirectory
        If (DirOrFile = vbDirectory) Then
            'ITS A FOLDER ADD THE ROOTS FOLDERS AND THE CONTENTS
            CutAndAddFolders lstFileList.Text, False
            FileAdder (lstFileList.Text & "\")
        ElseIf (DirOrFile <> vbDirectory) Then
            'ITS A FILE
            CutAndAddFolders lstFileList.Text, True
        End If
    Next i
    '
    If (Not LockAndBlock(UnitsRec, True)) Then
        PrimoSDK_CloseImage (Handle)
        bBusy = False
        EnableDisable (CurFunc)
    End If
    
    '
    ' WRITE OR TEST
    '
    dwErr = PrimoSDK_SaveImage(Handle, strFileName, Total)
    If (dwErr <> PRIMOSDK_OK) Then
        'AN ERROR HAPPENED
        DisplayError dwErr, "PrimoSDK_SaveImage", ""
        PrimoSDK_CloseImage (Handle)
        bBusy = False
        EnableDisable (CurFunc)
        LockAndBlock UnitsRec, False
    Else
        'WRITING STARTED LET THE SYSTEM GOP CONTROLLED BY THE TIMER
        txtResult.Text = Total & " secors are needed for these " & TotFileCount & " files"
        txtResult.Refresh
        Sleep (1000)
        ProgTimer.Enabled = True
    End If
    
End Function

Function AppFunc_RecordAudio()
'       ======================
'
' RECORD AN AUDIO DISC
'
'
Dim TotEntries As Long
Dim dwCloseDisc As Long
Dim dwFileToAdd As String
Dim dwSize As Long

    TotEntries = lstFileList.ListCount
    ' CHECK THAT THE FILE LIST IS NOT EMPTY
    If TotEntries = 0 Then
        MsgBox "The recording list is empty. Drag some file in it and try again.", vbInformation
        Exit Function
    End If
    
    ' SET THE RECORDING STATUS
    bBusy = True
    EnableDisable (CurFunc)
    If chkCloseDisc.Value = 1 Then dwCloseDisc = PRIMOSDK_CLOSEDISC Else dwCloseDisc = 0
    ' MAKE THE ENW IMAGE FOR THE DATA DISC
    dwErr = PrimoSDK_NewAudio(Handle, UnitsRec(1))
    If dwErr <> PRIMOSDK_OK Then
        DisplayError dwErr, "PrimoSDK_NewAudio", ""
        bBusy = False
        EnableDisable CurFunc
        Exit Function
    End If
    ' LOAD THE FILES IN THE IMAGE
    dwSize = 0
    i = 0
    For i = 0 To TotEntries - 1
        '
        lstFileList.ListIndex = i
        dwFileToAdd = lstFileList.Text
        dwErr = PrimoSDK_AddAudioTrack(Handle, dwFileToAdd, 150, dwSize)
        If (dwErr <> PRIMOSDK_OK) Then Exit For
        TotalSize = TotalSize + dwSize
    Next i
    
    If dwErr <> PRIMOSDK_OK Then
        DisplayError dwErr, "PrimoSDK_AddAudioTrack", ""
        PrimoSDK_CloseAudio (Handle)
        bBusy = False
        EnableDisable (CurFunc)
        PrimoSDK_CloseAudio (Handle)
    End If
    
    ' DISPLAY THE NEEDED SECTORS AND WAIT A LITTLE WHILE
    txtResult.Text = TotalSize & " sectors are needed for these " & i & " audio tracks..."
    Sleep (1000)
    '
    If Not LockAndBlock(UnitsRec, True) Then
        bBusy = False
        EnableDisable (CurFunc)
        PrimoSDK_CloseAudio (Handle)
        Exit Function
    End If
    
    ' WRITE OR TEST
    If chkCloseDisc.Value = 1 Then WAction = WAction + dwCloseDisc
    
    dwErr = PrimoSDK_WriteAudio(Handle, WAction, Speed)
    If (dwErr <> PRIMOSDK_OK) Then
        'DISPLAY THE ERROR
        DisplayError dwErr, "PrimoSDK_WriteAudio", ""
        bBusy = False
        EnableDisable CurFunc
        PrimoSDK_CloseAudio (Handle)
        LockAndBlock UnitsRec, False
    Else
        'THE WRITING HAS NOW BEGUN
        ProgTimer.Enabled = True
    End If
    
End Function
Function AppFunc_VideoCD()
'       ==================
'
' RECORD A VIDEO CD
'
'
Dim TotEntries As Long
Dim dwCloseDisc As Long
Dim dwFileToAdd As String
Dim dwSize As Long
Dim VCDDataFilePath As String

    TotEntries = lstFileList.ListCount
    ' CHECK THAT THE FILE LIST IS NOT EMPTY
    If TotEntries = 0 Then
        MsgBox "The recording list is empty. Drag some file in it and try again.", vbInformation
        Exit Function
    End If
    
    ' SET THE RECORDING STATUS
    bBusy = True
    EnableDisable (CurFunc)
    If chkCloseDisc.Value = 1 Then dwCloseDisc = PRIMOSDK_CLOSEDISC Else dwCloseDisc = 0
    

    ' SET THE TEMP FOLDER TO ROOT OF C:
    TempFolder = "C:\"
    VCDDataFilePath = App.Path & "\" & "Vcd.dta"
    
    
    
    ' MAKE THE NEW IMAGE FOR THE VIDEO DISC
    dwErr = PrimoSDK_NewVideoCD(Handle, UnitsRec(1), TempFolder, VCDDataFilePath)
    If dwErr <> PRIMOSDK_OK Then
        DisplayError dwErr, "PrimoSDK_NewVideoCD", ""
        bBusy = False
        EnableDisable CurFunc
        Exit Function
    End If
    ' LOAD THE FILES IN THE IMAGE
    dwSize = 0
    i = 0
    For i = 0 To TotEntries - 1
        '
        lstFileList.ListIndex = i
        dwFileToAdd = lstFileList.Text
        dwErr = PrimoSDK_AddVideoCDStream(Handle, dwFileToAdd, dwSize)
        If (dwErr <> PRIMOSDK_OK) Then Exit For
        TotalSize = TotalSize + dwSize
    Next i
    
    If dwErr <> PRIMOSDK_OK Then
        DisplayError dwErr, "PrimoSDK_AddVideoCDStream", ""
        PrimoSDK_CloseAudio (Handle)
        bBusy = False
        EnableDisable (CurFunc)
        PrimoSDK_CloseVideoCD (Handle)
    End If
    
    ' DISPLAY THE NEEDED SECTORS AND WAIT A LITTLE WHILE
    txtResult.Text = TotalSize & " sectors are needed for these " & i & " tracks..."
    Sleep (1000)
    '
    If Not LockAndBlock(UnitsRec, True) Then
        bBusy = False
        EnableDisable (CurFunc)
        PrimoSDK_CloseVideoCD (Handle)
        Exit Function
    End If
    
    ' WRITE OR TEST
    If chkCloseDisc.Value = 1 Then WAction = WAction + dwCloseDisc
    
    dwErr = PrimoSDK_WriteVideoCD(Handle, WAction, Speed)
    If (dwErr <> PRIMOSDK_OK) Then
        'DISPLAY THE ERROR
        DisplayError dwErr, "PrimoSDK_WriteAudio", ""
        bBusy = False
        EnableDisable CurFunc
        PrimoSDK_CloseVideoCD (Handle)
        LockAndBlock UnitsRec, False
    Else
        'THE WRITING HAS NOW BEGUN
        ProgTimer.Enabled = True
    End If
    

End Function


Function AppFunc_AudioExtract()
'       ======================
'
' RECORD AN AUDIO DISC
'
'
Dim dwCount As Long
Dim strFileName As String
Dim dwSize As Long

    '
    ' ASK THE FILE NAME
    '
    cdaDialog.DialogTitle = "Wave File Where to Extract"
    cdaDialog.Filter = "Wave File (*.wav)|*.wav|"
    cdaDialog.Flags = 2 + 4
    cdaDialog.ShowSave
    MainForm.Refresh
    If cdaDialog.Filename = "" Then Exit Function
    MainForm.Refresh
    If Right(UCase(cdaDialog.Filename), 4) <> ".WAV" Then
        strFileName = cdaDialog.Filename & ".wav"
    Else
        strFileName = cdaDialog.Filename
    End If
    
    bBusy = True
    EnableDisable CurFunc
    If Not (LockAndBlock(UnitsRec, True)) Then
        bBusy = False
        EnableDisable CurFunc
        Exit Function
    End If
    UnitSource = cmbSource.ItemData(cmbSource.ListIndex)
    dwErr = PrimoSDK_ExtractAudioTrack(Handle, UnitSource, dwCount, strFileName, dwSize)
    If (dwErr <> PRIMOSDK_OK) Then
        DisplayError dwErr, "PrimoSDK_ExtractAudioTrack", ""
        bBusy = False
        EnableDisable CurFunc
        LockAndBlock UnitsRec, False
    Else
        ' WRITING STARTED, START THE TIMER
        txtResult.Text = dwSize & " sectors to Extract..."
        Sleep (1000)
        ProgTimer.Enabled = True
    End If
    
End Function

Function AppFunc_EraseDisc()
'       ===================
'
' ERASE DISC
'
'
Dim dwUnit As Long
Dim MediumType As Long
Dim dwMediumFormat As Long
Dim dwErasable As Long
Dim Tracks As Long
Dim dwUsed As Long
Dim Free As Long

    '
    ' ERASE DISC
    '
    dwUnit = cmbRecorder.ItemData(cmbRecorder.ListIndex)
    If dwUnit = &HFFFFFFFF Then Exit Function
    dwErr = PrimoSDK_DiscInfo(Handle, dwUnit, MediumType, dwMediumFormat, dwErasable, Tracks, dwUsed, Free)
    If (Reply <> PRIMOSDK_OK) Then
        DisplayError dwErr, "PrimoSDK_DiscInfo", ""
        Exit Function
    End If
    If dwErasable = 0 Then
        MsgBox "The recorder is not RW and/or the medium is not erasable.", vbInformation
        Exit Function
    End If
    'SAFETY MESSAGE
    Reply = MsgBox("Are you sure you want to erase this rewritable disc?", vbYesNo + vbInformation)
    If (Reply <> vbNo) Then
    'START THE ERASE PROCESS
        If Not LockAndBlock(UnitsRec, True) Then Exit Function
        dwErr = PrimoSDK_EraseMedium(Handle, dwUnit, PRIMOSDK_ERASEQUICK)
        If (dwErr <> PRIMOSDK_OK) Then
            'AND ERROR HAPPENED IMMEDIATELY DISPALY IT
            DisplayError dwErr, "PrimoSDK_EraseMedium", ""
            bBusy = False
            EnableDisable CurFunc
            LockAndBlock UnitsRec, False
        Else
            bBusy = True
            EnableDisable (CurFunc)
            'THE ERASE STARTED OK, LET THE SYSTEM GO CONTROLLED BY THE TIMER
            ProgTimer.Enabled = True
        End If
    End If

End Function

Function VBStr(Stri As String) As String
'       =======
'
' RETURN THE C STRING OUT OF A VB STRING WITH THE ENDING <00>
'
    VBStr = Left$(Stri$, InStr(Stri$, Chr$(0)) - 1)

End Function

Function LockAndBlock(pdwUnit() As Long, bWAction As Boolean) As Boolean
'       ==============
'
'  LOCK AND BLOCK THE AIN ON THE PASSED UNITS. WHILE LOCKING, IF THE
'   AIN SERVICE IS NOT PRESENT, IT PRESENTS A MESSAGE AND THE USER
'    CAN STILL ABORT THE OPERATION.
'
Dim dwUnit As Long

    For i = 1 To 64
        If (pdwUnit(i) = -1) Then Exit For
        dwUnit = pdwUnit(i)
        If (bWAction) Then Act = PRIMOSDK_LOCK Else Act = PRIMOSDK_UNLOCK
        '
        dwret = PrimoSDK_UnitAIN(Handle, dwUnit, Act)
        If (dwret = PRIMOSDK_NOAINCONTROL And bWAction) Then
            If (MsgBox("The File System and Auto Insert Notification blocking is not active, probably because we are using WinASPI." & Chr(10) & "You need to have the Auto Insert Notification turned off on all drives engaged by this operation." & Chr(10) & "Also, it is better that you do not access these drives with the Explorer or any other application, until this operation terminates." & Chr(10) & Chr(10) & "Continue?", vbYesNo + vbQuestion) = vbNo) Then
                LockAndBlock = False
                Exit Function
            End If
        End If

        dwret = PrimoSDK_UnitLock(Handle, dwUnit, Act)
    Next i
    '
    LockAndBlock = True
    
End Function

Function LockAndBlockSource(pdwUnit As Long, bWAction As Boolean) As Boolean
'       ====================
'
'  LOCK AND BLOCK THE AIN ON THE PASSED UNITS. WHILE LOCKING, IF THE
'   AIN SERVICE IS NOT PRESENT, IT PRESENTS A MESSAGE AND THE USER
'    CAN STILL ABORT THE OPERATION.
'
Dim dwUnit As Long

    dwUnit = UnitSource
    If (bWAction) Then Act = PRIMOSDK_LOCK Else Act = PRIMOSDK_UNLOCK

    dwret = PrimoSDK_UnitAIN(Handle, dwUnit, Act)
    If (dwret = PRIMOSDK_NOAINCONTROL And bWAction) Then
        If (MsgBox("The File System and Auto Insert Notification blocking is not active, probably because we are using WinASPI." & Chr(10) & "You need to have the Auto Insert Notification turned off on all drives engaged by this operation." & Chr(10) & "Also, it is better that you do not access these drives with the Explorer or any other application, until this operation terminates." & Chr(10) & Chr(10) & "Continue?", vbYesNo) = vbNo) Then
            LockAndBlockSource = False
            Exit Function
        End If
    End If

    dwret = PrimoSDK_UnitLock(Handle, dwUnit, Act)
    LockAndBlockSource = True

End Function

Sub DisplayError(ByVal dwError As Long, ByVal FuncName As String, AddPar As String)
'  ==============
'
' DISPLAYS THE APPROPRIATE ERROR ACCORDING TO THE GIVEN VALUE
'
    Select Case dwError
    Case 1
        Str3$ = "PRIMOSDK_CMDSEQUENCE"
    Case 2
        Str3$ = "PRIMOSDK_NOASPI"
    Case 3
        Str3$ = "PRIMOSDK_INTERR"
    Case 4
        Str3$ = "PRIMOSDK_BADPARAM"
    Case 6
        Str3$ = "PRIMOSDK_ALREADYEXIST"
    Case 7
        Str3$ = "PRIMOSDK_NOTREADABLE"
    Case 8
        Str3$ = "PRIMOSDK_NOSPACE"
    Case 9
        Str3$ = "PRIMOSDK_INVALIDMEDIUM"
    Case 10
        Str3$ = "PRIMOSDK_RUNNING"
    Case 11
        Str3$ = "PRIMOSDK_BUR"
    Case 12
        Str3$ = "PRIMOSDK_SCSIERROR"
    Case 13
        Str3$ = "PRIMOSDK_UNITERROR"
    Case 14
        Str3$ = "PRIMOSDK_NOTREADY"
    Case 15
        Str3$ = "PRIMOSDK_DISKOVERFLOW"
    Case 16
        Str3$ = "PRIMOSDK_INVALIDSOURCE"
    Case 17
        Str3$ = "PRIMOSDK_INCOMPATIBLE"
    Case 18
        Str3$ = "PRIMOSDK_FILEERROR"
    Case 23
        Str3$ = "PRIMOSDK_ITSADEMO"
    Case 24
        Str3$ = "PRIMOSDK_USERABORT"
    Case 25
        Str3$ = "PRIMOSDK_BADHANDLE"
    Case 26
        Str3$ = "PRIMOSDK_BADUNIT"
    Case 27
        Str3$ = "PRIMOSDK_ERRORLOADING"
    Case 29
        Str3$ = "PRIMOSDK_NOAINCONTROL"
    Case 30
        Str3$ = "PRIMOSDK_READERROR"
    Case 31
        Str3$ = "PRIMOSDK_WRITEERROR"
    End Select
    
    Str2$ = "The function """ + FuncName + """ returned  " + Format(dwError) + "  (" + Str3$ + ")."
    If (AddPar <> "") Then Str2$ = Str2$ + vbLf + vbLf + AddPar
    '
    MsgBox Str2$, vbInformation + vbOKOnly
    EnableDisable CurFunc
   
End Sub

Public Function FileAdder(DirPath As String)
'     ==========
'
' RECURSIVE PROCEDURE. IT ADDS THE FILES OF A DIRECTORY TO THE DATA IMAGE.
' (NAME OF THE DIRECTORY MUST HAVE A "/" AT THE END)
'
' PARAMETERS: szPath IS THE NAME OF THE CURRENT FILE OR DIRECTORY
'             TO ADD IN THE IMAGE (WILDCARDS OK)
'
' RETURNS: AN ERROR CODE
'
Dim MyFileName As String
Dim FolderList As String
Dim MyDirName As String
Dim NewFolder As String

    '
    ' NORMALIZE AND ADD THE MAIN DIR TO THE TOOLKIT
    '
    AddThis = SkipDriveLetter(DirPath)
    dwret = PrimoSDK_AddFolder(Handle, AddThis)
    
    '
    ' LOOP FOR ANY DIRECTORIES HERE
    '
    MyDirName = Dir(DirPath, vbDirectory)
    FolderList = ""
    'START THE LOOP
    Do While MyDirName <> ""
        'DOUBLE CHECK THAT THE ENTRY IS A DIRECTORY
        If MyDirName <> "." And MyDirName <> ".." Then
            If (GetAttr(DirPath & MyDirName) And vbDirectory) = vbDirectory Then
                'ADD ALL OF THE DIRECTORIES IN THE LIST AND TO THE TOOLKIT
                FolderList = FolderList + MyDirName + vbCr
                'List1.AddItem DirPath + MyDirName
                AddThis = SkipDriveLetter(DirPath)
                txtResult.Text = "Adding: " & DirPath & MyDirName
                txtResult.Refresh
                dwret = PrimoSDK_AddFolder(Handle, AddThis)
                If (dwret <> PRIMOSDK_OK And dwret <> PRIMOSDK_ALREADYEXIST) Then
                    DisplayError dwret, "PrimoSDK_AddFolder", ""
                    PrimoSDK_CloseImage (Handle)
                    bBusy = False
                    EnableDisable (CurFunc)
                    Exit Function
                End If
            End If
        End If
        MyDirName = Dir
    Loop
    
    '
    ' LOOP FOR EVERY FILE
    '
    MyFileName = Dir(DirPath, vbArchive + vbHidden + vbNormal + vbReadOnly + vbSystem)
    Do While MyFileName <> ""
        If (Not bStop) Then
            'ADD ALL OF THE FILES TO THE TOOLKIT
            txtResult.Text = "Adding " & DirPath & MyFileName
            AddThis = SkipDriveLetter(DirPath & MyFileName)
            TotFileCount = TotFileCount + 1
            dwret = PrimoSDK_AddFile(Handle, AddThis, DirPath & MyFileName)
            txtResult.Text = "Adding (" & TotFileCount & ") " & DirPath & MyDirName & MyFileName
            txtResult.Refresh
            If (dwret <> PRIMOSDK_OK And dwret <> PRIMOSDK_ALREADYEXIST) Then
                DisplayError dwret, "PrimoSDK_AddFile", ""
                PrimoSDK_CloseImage (Handle)
                bBusy = False
                EnableDisable (CurFunc)
                Exit Function
            End If
        End If
        MyFileName = Dir
    Loop
    
    '
    ' DO THE RECURSION FOR EVERY FOLDER WE FOUND BEFORE
    '
    Do While FolderList <> ""
        k = InStr(FolderList, vbCr)
        NewFolder = Left(FolderList, k)
        FolderList = Right(FolderList, Len(FolderList) - Len(NewFolder))
        NewFolder = Left(NewFolder, Len(NewFolder) - 1)
        NewFolder = DirPath & NewFolder & "\"
        FileAdder (NewFolder)
    Loop
         
    txtResult.Refresh
         
End Function

Private Sub sliSwap_Change()
'
' DISPLAY THE RIGHT SCROLL VALUE
'
    If sliSwap.Value = 0 Then
        lblSwapResult.Caption = "None"
    ElseIf sliSwap.Value = 257 Then
        lblSwapResult = "All"
    ElseIf sliSwap.Value < 256 Then
        lblSwapResult = sliSwap.Value & "KB"
    End If
    
End Sub

Private Function SkipDriveLetter(ByVal StringName As String) As String
'
' SKIP THE DRIVE LETTER AND THE PATH UP TO THE FOURTH "\" IF UNC PATH FOUND
'
    Drive = Left(StringName, 2)
    If Right(Drive, 1) = ":" Then
        ' IS A PATH
        SkipDriveLetter = Right(StringName, Len(StringName) - 2)
    ElseIf Left(Drive, 1) = "\" Then
        ' IS A SERVER OR NETWORK FILE/FOLDER
        For i = 1 To 4
            k = InStr(StringName, "\")
            StringName = Right(StringName, Len(StringName) - k)
        Next i
        SkipDriveLetter = "\" & StringName
    End If

End Function

Function CutAndAddFolders(DirPath As String, IsItAFile As Boolean)
'
' ADD ALL THE INNER FOLDERS UP TO THE FINAL ONE AND IF A FILE ADD IT
'
Dim Pos As Long
Dim AddThis As String
Dim TempPath As String
Dim MyFilePath As String
    
    MyFilePath = SkipDriveLetter(DirPath)
    MyFilePath = Right(MyFilePath, Len(MyFilePath) - 1)
    Do
        Pos = InStr(Pos + 1, MyFilePath, "\")
        If (Pos = 0) Then Exit Do
        AddThis = Left(MyFilePath, Pos)
        AddThis = "\" & AddThis
        'ADD THE FOLDER TO THE TOOLKIT
        dwret = PrimoSDK_AddFolder(Handle, AddThis)
        If (dwret <> PRIMOSDK_OK And dwret <> PRIMOSDK_ALREADYEXIST) Then
            DisplayError dwret, "PrimoSDK_AddFolder", ""
            PrimoSDK_CloseImage (Handle)
            bBusy = False
            EnableDisable (CurFunc)
            Exit Function
        End If
    Loop Until False
        
    MyFilePath = "\" & MyFilePath
    If IsItAFile = True Then
        TotFileCount = TotFileCount + 1
        dwret = PrimoSDK_AddFile(Handle, MyFilePath, DirPath)
        If (dwret <> PRIMOSDK_OK And dwret <> PRIMOSDK_ALREADYEXIST) Then
            DisplayError dwret, "PrimoSDK_AddFolder", ""
            PrimoSDK_CloseImage (Handle)
            bBusy = False
            EnableDisable (CurFunc)
            Exit Function
        End If
    End If
        
    txtResult.Text = "Pre-Mastering..."
    txtResult.Refresh
        
End Function

