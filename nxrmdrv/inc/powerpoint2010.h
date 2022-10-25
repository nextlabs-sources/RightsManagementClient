﻿#pragma once
#pragma pack(push, 8)

#include <comdef.h>

namespace PowerPoint2010 {

//
// Forward references and typedefs
//

struct __declspec(uuid("91493440-5a91-11cf-8700-00aa0060263b"))
/* LIBID */ __PowerPoint;
enum PpWindowState;
enum PpArrangeStyle;
enum PpViewType;
enum PpColorSchemeIndex;
enum PpSlideSizeType;
enum PpSaveAsFileType;
enum PpTextStyleType;
enum PpSlideLayout;
enum PpEntryEffect;
enum PpTextLevelEffect;
enum PpTextUnitEffect;
enum PpChartUnitEffect;
enum PpAfterEffect;
enum PpAdvanceMode;
enum PpSoundEffectType;
enum PpFollowColors;
enum PpUpdateOption;
enum PpParagraphAlignment;
enum PpBaselineAlignment;
enum PpTabStopType;
enum PpIndentControl;
enum PpChangeCase;
enum PpSlideShowPointerType;
enum PpSlideShowState;
enum PpSlideShowAdvanceMode;
enum PpFileDialogType;
enum PpPrintOutputType;
enum PpPrintHandoutOrder;
enum PpPrintColorType;
enum PpSelectionType;
enum PpDirection;
enum PpDateTimeFormat;
enum PpTransitionSpeed;
enum PpMouseActivation;
enum PpActionType;
enum PpPlaceholderType;
enum PpSlideShowType;
enum PpPrintRangeType;
enum PpAutoSize;
enum PpMediaType;
enum PpSoundFormatType;
enum PpFarEastLineBreakLevel;
enum PpSlideShowRangeType;
enum PpFrameColors;
struct __declspec(uuid("91493450-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Collection;
struct __declspec(uuid("91493442-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ _Application;
struct __declspec(uuid("91493451-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ _Global;
struct /* coclass */ Application;
struct /* coclass */ Global;
struct __declspec(uuid("91493452-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ ColorFormat;
struct __declspec(uuid("91493453-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ SlideShowWindow;
struct __declspec(uuid("91493454-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Selection;
struct __declspec(uuid("91493455-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ DocumentWindows;
struct __declspec(uuid("91493456-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ SlideShowWindows;
struct __declspec(uuid("91493457-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ DocumentWindow;
struct __declspec(uuid("91493458-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ View;
struct __declspec(uuid("91493459-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ SlideShowView;
struct __declspec(uuid("9149345a-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ SlideShowSettings;
struct __declspec(uuid("9149345b-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ NamedSlideShows;
struct __declspec(uuid("9149345c-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ NamedSlideShow;
struct __declspec(uuid("9149345d-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ PrintOptions;
struct __declspec(uuid("9149345e-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ PrintRanges;
struct __declspec(uuid("9149345f-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ PrintRange;
struct __declspec(uuid("91493460-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ AddIns;
struct __declspec(uuid("91493461-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ AddIn;
struct __declspec(uuid("91493462-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Presentations;
struct __declspec(uuid("91493463-5a91-11cf-8700-00aa0060263b"))
/* interface */ PresEvents;
struct /* coclass */ Presentation;
struct __declspec(uuid("91493464-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Hyperlinks;
struct __declspec(uuid("91493465-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Hyperlink;
struct __declspec(uuid("91493466-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ PageSetup;
struct __declspec(uuid("91493467-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Fonts;
struct __declspec(uuid("91493468-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ ExtraColors;
struct __declspec(uuid("91493469-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Slides;
struct __declspec(uuid("9149346a-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ _Slide;
struct __declspec(uuid("9149346b-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ SlideRange;
struct __declspec(uuid("9149346c-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ _Master;
struct __declspec(uuid("9149346d-5a91-11cf-8700-00aa0060263b"))
/* interface */ SldEvents;
struct /* coclass */ Slide;
struct __declspec(uuid("9149346e-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ ColorSchemes;
struct __declspec(uuid("9149346f-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ ColorScheme;
struct __declspec(uuid("91493470-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ RGBColor;
struct __declspec(uuid("91493471-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ SlideShowTransition;
struct __declspec(uuid("91493472-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ SoundEffect;
struct __declspec(uuid("91493473-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ SoundFormat;
struct __declspec(uuid("91493474-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ HeadersFooters;
struct __declspec(uuid("91493475-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Shapes;
struct __declspec(uuid("91493476-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Placeholders;
struct __declspec(uuid("91493477-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ PlaceholderFormat;
struct __declspec(uuid("91493478-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ FreeformBuilder;
struct __declspec(uuid("91493479-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Shape;
struct __declspec(uuid("9149347a-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ ShapeRange;
struct __declspec(uuid("9149347b-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ GroupShapes;
struct __declspec(uuid("9149347c-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Adjustments;
struct __declspec(uuid("9149347d-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ PictureFormat;
struct __declspec(uuid("9149347e-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ FillFormat;
struct __declspec(uuid("9149347f-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ LineFormat;
struct __declspec(uuid("91493480-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ ShadowFormat;
struct __declspec(uuid("91493481-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ ConnectorFormat;
struct __declspec(uuid("91493482-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ TextEffectFormat;
struct __declspec(uuid("91493483-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ ThreeDFormat;
struct __declspec(uuid("91493484-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ TextFrame;
struct __declspec(uuid("91493485-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ CalloutFormat;
struct __declspec(uuid("91493486-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ ShapeNodes;
struct __declspec(uuid("91493487-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ ShapeNode;
struct __declspec(uuid("91493488-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ OLEFormat;
struct __declspec(uuid("91493489-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ LinkFormat;
struct __declspec(uuid("9149348a-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ ObjectVerbs;
struct __declspec(uuid("9149348b-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ AnimationSettings;
struct __declspec(uuid("9149348c-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ ActionSettings;
struct __declspec(uuid("9149348d-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ ActionSetting;
struct __declspec(uuid("9149348e-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ PlaySettings;
struct __declspec(uuid("9149348f-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ TextRange;
struct __declspec(uuid("91493490-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Ruler;
struct __declspec(uuid("91493491-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ RulerLevels;
struct __declspec(uuid("91493492-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ RulerLevel;
struct __declspec(uuid("91493493-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ TabStops;
struct __declspec(uuid("91493494-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ TabStop;
struct __declspec(uuid("91493495-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Font;
struct __declspec(uuid("91493496-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ ParagraphFormat;
struct __declspec(uuid("91493497-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ BulletFormat;
struct __declspec(uuid("91493498-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ TextStyles;
struct __declspec(uuid("91493499-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ TextStyle;
struct __declspec(uuid("9149349a-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ TextStyleLevels;
struct __declspec(uuid("9149349b-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ TextStyleLevel;
struct __declspec(uuid("9149349c-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ HeaderFooter;
struct __declspec(uuid("9149349d-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ _Presentation;
struct __declspec(uuid("914934b9-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Tags;
struct __declspec(uuid("914934be-5a91-11cf-8700-00aa0060263b"))
/* interface */ MouseTracker;
struct __declspec(uuid("914934bf-5a91-11cf-8700-00aa0060263b"))
/* interface */ MouseDownHandler;
struct __declspec(uuid("914934c0-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ OCXExtender;
struct __declspec(uuid("914934c1-5a91-11cf-8700-00aa0060263b"))
/* interface */ OCXExtenderEvents;
struct /* coclass */ OLEControl;
enum PpBorderType;
enum PpHTMLVersion;
enum PpPublishSourceType;
enum PpBulletType;
enum PpNumberedBulletStyle;
enum PpShapeFormat;
enum PpExportMode;
struct __declspec(uuid("914934c2-5a91-11cf-8700-00aa0060263b"))
/* interface */ EApplication;
struct __declspec(uuid("914934c3-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Table;
struct __declspec(uuid("914934c4-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Columns;
struct __declspec(uuid("914934c5-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Column;
struct __declspec(uuid("914934c6-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Rows;
struct __declspec(uuid("914934c7-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Row;
struct __declspec(uuid("914934c8-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ CellRange;
struct __declspec(uuid("914934c9-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Cell;
struct __declspec(uuid("914934ca-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Borders;
struct __declspec(uuid("914934cb-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Panes;
struct __declspec(uuid("914934cc-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Pane;
struct __declspec(uuid("914934cd-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ DefaultWebOptions;
struct __declspec(uuid("914934ce-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ WebOptions;
struct __declspec(uuid("914934cf-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ PublishObjects;
struct __declspec(uuid("914934d0-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ PublishObject;
struct __declspec(uuid("914934d2-5a91-11cf-8700-00aa0060263b"))
/* interface */ MasterEvents;
struct /* coclass */ Master;
enum PpPasteDataType;
struct __declspec(uuid("914934d3-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ _PowerRex;
struct /* coclass */ PowerRex;
struct __declspec(uuid("914934d4-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Comments;
struct __declspec(uuid("914934d5-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Comment;
struct __declspec(uuid("914934d6-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Designs;
struct __declspec(uuid("914934d7-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Design;
struct __declspec(uuid("914934d8-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ DiagramNode;
struct __declspec(uuid("914934d9-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ DiagramNodeChildren;
struct __declspec(uuid("914934da-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ DiagramNodes;
struct __declspec(uuid("914934db-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Diagram;
struct __declspec(uuid("914934dc-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ TimeLine;
struct __declspec(uuid("914934dd-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Sequences;
enum MsoAnimEffect;
enum MsoAnimateByLevel;
enum MsoAnimTriggerType;
enum MsoAnimAfterEffect;
enum MsoAnimTextUnitEffect;
struct __declspec(uuid("914934de-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Sequence;
enum MsoAnimEffectRestart;
enum MsoAnimEffectAfter;
struct __declspec(uuid("914934df-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Effect;
struct __declspec(uuid("914934e0-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Timing;
enum MsoAnimDirection;
struct __declspec(uuid("914934e1-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ EffectParameters;
struct __declspec(uuid("914934e2-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ EffectInformation;
enum MsoAnimType;
struct __declspec(uuid("914934e3-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ AnimationBehaviors;
enum MsoAnimAdditive;
enum MsoAnimAccumulate;
struct __declspec(uuid("914934e4-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ AnimationBehavior;
struct __declspec(uuid("914934e5-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ MotionEffect;
struct __declspec(uuid("914934e6-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ ColorEffect;
struct __declspec(uuid("914934e7-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ ScaleEffect;
struct __declspec(uuid("914934e8-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ RotationEffect;
enum MsoAnimProperty;
struct __declspec(uuid("914934e9-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ PropertyEffect;
struct __declspec(uuid("914934ea-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ AnimationPoints;
struct __declspec(uuid("914934eb-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ AnimationPoint;
enum PpAlertLevel;
struct __declspec(uuid("914934ec-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ CanvasShapes;
enum PpRevisionInfo;
struct __declspec(uuid("914934ed-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ AutoCorrect;
struct __declspec(uuid("914934ee-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Options;
enum MsoAnimCommandType;
struct __declspec(uuid("914934ef-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ CommandEffect;
enum MsoAnimFilterEffectType;
enum MsoAnimFilterEffectSubtype;
struct __declspec(uuid("914934f0-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ FilterEffect;
struct __declspec(uuid("914934f1-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ SetEffect;
struct __declspec(uuid("914934f2-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ CustomLayouts;
struct __declspec(uuid("914934f3-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ CustomLayout;
enum PpRemoveDocInfoType;
enum PpCheckInVersionType;
enum MsoClickState;
enum PpFixedFormatType;
enum PpFixedFormatIntent;
struct __declspec(uuid("914934f5-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ TableStyle;
struct __declspec(uuid("914934f6-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ CustomerData;
struct __declspec(uuid("914934f7-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ Research;
struct __declspec(uuid("914934f8-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ TableBackground;
struct __declspec(uuid("914934f9-5a91-11cf-8700-00aa0060263b"))
/* dual interface */ TextFrame2;
struct __declspec(uuid("92d41a50-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ FileConverters;
struct __declspec(uuid("92d41a51-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ FileConverter;
enum XlBackground;
enum XlChartGallery;
enum XlChartPicturePlacement;
enum XlDataLabelSeparator;
enum XlPattern;
enum XlPictureAppearance;
enum XlCopyPictureFormat;
enum XlRgbColor;
enum XlLineStyle;
enum XlAxisCrosses;
enum XlAxisGroup;
enum XlAxisType;
enum XlBarShape;
enum XlBorderWeight;
enum XlCategoryType;
enum XlChartElementPosition;
enum XlChartItem;
enum XlOrientation;
enum XlChartPictureType;
enum XlChartSplitType;
enum XlColorIndex;
enum XlConstants;
enum XlDataLabelPosition;
enum XlDataLabelsType;
enum XlDisplayBlanksAs;
enum XlDisplayUnit;
enum XlEndStyleCap;
enum XlErrorBarDirection;
enum XlErrorBarInclude;
enum XlErrorBarType;
enum XlHAlign;
enum XlLegendPosition;
enum XlMarkerStyle;
enum XlPivotFieldOrientation;
enum XlReadingOrder;
enum XlRowCol;
enum XlScaleType;
enum XlSizeRepresents;
enum XlTickLabelOrientation;
enum XlTickLabelPosition;
enum XlTickMark;
enum XlTimeUnit;
enum XlTrendlineType;
enum XlUnderlineStyle;
enum XlVAlign;
struct __declspec(uuid("92d41a52-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ Axes;
struct __declspec(uuid("92d41a53-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ Axis;
struct __declspec(uuid("92d41a54-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ AxisTitle;
struct __declspec(uuid("92d41a55-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ Chart;
struct __declspec(uuid("92d41a56-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ ChartBorder;
struct __declspec(uuid("92d41a57-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ ChartCharacters;
struct __declspec(uuid("92d41a58-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ ChartArea;
struct __declspec(uuid("92d41a59-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ ChartColorFormat;
struct __declspec(uuid("92d41a5a-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ ChartData;
struct __declspec(uuid("92d41a5b-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ ChartFillFormat;
struct __declspec(uuid("92d41a5c-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ ChartFormat;
struct __declspec(uuid("92d41a5d-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ ChartGroup;
struct __declspec(uuid("92d41a5e-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ ChartGroups;
struct __declspec(uuid("92d41a5f-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ ChartTitle;
struct __declspec(uuid("92d41a60-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ Corners;
struct __declspec(uuid("92d41a61-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ DataLabel;
struct __declspec(uuid("92d41a62-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ DataLabels;
struct __declspec(uuid("92d41a63-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ DataTable;
struct __declspec(uuid("92d41a64-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ DisplayUnitLabel;
struct __declspec(uuid("92d41a65-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ DownBars;
struct __declspec(uuid("92d41a66-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ DropLines;
struct __declspec(uuid("92d41a67-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ ErrorBars;
struct __declspec(uuid("92d41a68-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ Floor;
struct __declspec(uuid("92d41a69-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ ChartFont;
struct __declspec(uuid("92d41a6a-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ Gridlines;
struct __declspec(uuid("92d41a6b-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ HiLoLines;
struct __declspec(uuid("92d41a6c-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ Interior;
struct __declspec(uuid("92d41a6d-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ LeaderLines;
struct __declspec(uuid("92d41a6e-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ Legend;
struct __declspec(uuid("92d41a6f-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ LegendEntries;
struct __declspec(uuid("92d41a70-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ LegendEntry;
struct __declspec(uuid("92d41a71-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ LegendKey;
struct __declspec(uuid("92d41a72-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ PlotArea;
struct __declspec(uuid("92d41a73-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ Point;
struct __declspec(uuid("92d41a74-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ Points;
struct __declspec(uuid("92d41a75-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ Series;
struct __declspec(uuid("92d41a76-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ SeriesCollection;
struct __declspec(uuid("92d41a77-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ SeriesLines;
struct __declspec(uuid("92d41a78-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ TickLabels;
struct __declspec(uuid("92d41a79-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ Trendline;
struct __declspec(uuid("92d41a7a-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ Trendlines;
struct __declspec(uuid("92d41a7b-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ UpBars;
struct __declspec(uuid("92d41a7c-f07e-4ca4-af6f-bef486aa4e6f"))
/* dual interface */ Walls;
enum PpResampleMediaProfile;
enum PpMediaTaskStatus;
struct __declspec(uuid("ba72e550-4ff5-48f4-8215-5505f990966f"))
/* dual interface */ MediaFormat;
struct __declspec(uuid("ba72e551-4ff5-48f4-8215-5505f990966f"))
/* dual interface */ SectionProperties;
enum PpPlayerState;
struct __declspec(uuid("ba72e552-4ff5-48f4-8215-5505f990966f"))
/* dual interface */ Player;
struct __declspec(uuid("ba72e553-4ff5-48f4-8215-5505f990966f"))
/* dual interface */ ResampleMediaTask;
struct __declspec(uuid("ba72e554-4ff5-48f4-8215-5505f990966f"))
/* dual interface */ ResampleMediaTasks;
struct __declspec(uuid("ba72e555-4ff5-48f4-8215-5505f990966f"))
/* dual interface */ MediaBookmark;
struct __declspec(uuid("ba72e556-4ff5-48f4-8215-5505f990966f"))
/* dual interface */ MediaBookmarks;
struct __declspec(uuid("ba72e557-4ff5-48f4-8215-5505f990966f"))
/* dual interface */ Coauthoring;
struct __declspec(uuid("ba72e558-4ff5-48f4-8215-5505f990966f"))
/* dual interface */ Broadcast;
enum XlPieSliceLocation;
enum XlPieSliceIndex;
enum PpProtectedViewCloseReason;
struct __declspec(uuid("ba72e559-4ff5-48f4-8215-5505f990966f"))
/* dual interface */ ProtectedViewWindows;
struct __declspec(uuid("ba72e55a-4ff5-48f4-8215-5505f990966f"))
/* dual interface */ ProtectedViewWindow;

//
// Smart pointer typedef declarations
//

_COM_SMARTPTR_TYPEDEF(Collection, __uuidof(Collection));
_COM_SMARTPTR_TYPEDEF(ColorFormat, __uuidof(ColorFormat));
_COM_SMARTPTR_TYPEDEF(NamedSlideShow, __uuidof(NamedSlideShow));
_COM_SMARTPTR_TYPEDEF(NamedSlideShows, __uuidof(NamedSlideShows));
_COM_SMARTPTR_TYPEDEF(PrintRange, __uuidof(PrintRange));
_COM_SMARTPTR_TYPEDEF(PrintRanges, __uuidof(PrintRanges));
_COM_SMARTPTR_TYPEDEF(PrintOptions, __uuidof(PrintOptions));
_COM_SMARTPTR_TYPEDEF(AddIn, __uuidof(AddIn));
_COM_SMARTPTR_TYPEDEF(AddIns, __uuidof(AddIns));
_COM_SMARTPTR_TYPEDEF(PresEvents, __uuidof(PresEvents));
_COM_SMARTPTR_TYPEDEF(Presentations, __uuidof(Presentations));
_COM_SMARTPTR_TYPEDEF(Hyperlink, __uuidof(Hyperlink));
_COM_SMARTPTR_TYPEDEF(Hyperlinks, __uuidof(Hyperlinks));
_COM_SMARTPTR_TYPEDEF(PageSetup, __uuidof(PageSetup));
_COM_SMARTPTR_TYPEDEF(ExtraColors, __uuidof(ExtraColors));
_COM_SMARTPTR_TYPEDEF(SldEvents, __uuidof(SldEvents));
_COM_SMARTPTR_TYPEDEF(RGBColor, __uuidof(RGBColor));
_COM_SMARTPTR_TYPEDEF(ColorScheme, __uuidof(ColorScheme));
_COM_SMARTPTR_TYPEDEF(ColorSchemes, __uuidof(ColorSchemes));
_COM_SMARTPTR_TYPEDEF(SoundEffect, __uuidof(SoundEffect));
_COM_SMARTPTR_TYPEDEF(SlideShowTransition, __uuidof(SlideShowTransition));
_COM_SMARTPTR_TYPEDEF(SoundFormat, __uuidof(SoundFormat));
_COM_SMARTPTR_TYPEDEF(PlaceholderFormat, __uuidof(PlaceholderFormat));
_COM_SMARTPTR_TYPEDEF(Adjustments, __uuidof(Adjustments));
_COM_SMARTPTR_TYPEDEF(PictureFormat, __uuidof(PictureFormat));
_COM_SMARTPTR_TYPEDEF(FillFormat, __uuidof(FillFormat));
_COM_SMARTPTR_TYPEDEF(LineFormat, __uuidof(LineFormat));
_COM_SMARTPTR_TYPEDEF(ShadowFormat, __uuidof(ShadowFormat));
_COM_SMARTPTR_TYPEDEF(TextEffectFormat, __uuidof(TextEffectFormat));
_COM_SMARTPTR_TYPEDEF(ThreeDFormat, __uuidof(ThreeDFormat));
_COM_SMARTPTR_TYPEDEF(CalloutFormat, __uuidof(CalloutFormat));
_COM_SMARTPTR_TYPEDEF(ShapeNode, __uuidof(ShapeNode));
_COM_SMARTPTR_TYPEDEF(ShapeNodes, __uuidof(ShapeNodes));
_COM_SMARTPTR_TYPEDEF(LinkFormat, __uuidof(LinkFormat));
_COM_SMARTPTR_TYPEDEF(ObjectVerbs, __uuidof(ObjectVerbs));
_COM_SMARTPTR_TYPEDEF(OLEFormat, __uuidof(OLEFormat));
_COM_SMARTPTR_TYPEDEF(ActionSetting, __uuidof(ActionSetting));
_COM_SMARTPTR_TYPEDEF(ActionSettings, __uuidof(ActionSettings));
_COM_SMARTPTR_TYPEDEF(PlaySettings, __uuidof(PlaySettings));
_COM_SMARTPTR_TYPEDEF(AnimationSettings, __uuidof(AnimationSettings));
_COM_SMARTPTR_TYPEDEF(RulerLevel, __uuidof(RulerLevel));
_COM_SMARTPTR_TYPEDEF(RulerLevels, __uuidof(RulerLevels));
_COM_SMARTPTR_TYPEDEF(TabStop, __uuidof(TabStop));
_COM_SMARTPTR_TYPEDEF(TabStops, __uuidof(TabStops));
_COM_SMARTPTR_TYPEDEF(Ruler, __uuidof(Ruler));
_COM_SMARTPTR_TYPEDEF(Font, __uuidof(Font));
_COM_SMARTPTR_TYPEDEF(Fonts, __uuidof(Fonts));
_COM_SMARTPTR_TYPEDEF(HeaderFooter, __uuidof(HeaderFooter));
_COM_SMARTPTR_TYPEDEF(HeadersFooters, __uuidof(HeadersFooters));
_COM_SMARTPTR_TYPEDEF(Tags, __uuidof(Tags));
_COM_SMARTPTR_TYPEDEF(MouseTracker, __uuidof(MouseTracker));
_COM_SMARTPTR_TYPEDEF(MouseDownHandler, __uuidof(MouseDownHandler));
_COM_SMARTPTR_TYPEDEF(OCXExtender, __uuidof(OCXExtender));
_COM_SMARTPTR_TYPEDEF(OCXExtenderEvents, __uuidof(OCXExtenderEvents));
_COM_SMARTPTR_TYPEDEF(BulletFormat, __uuidof(BulletFormat));
_COM_SMARTPTR_TYPEDEF(ParagraphFormat, __uuidof(ParagraphFormat));
_COM_SMARTPTR_TYPEDEF(TextStyleLevel, __uuidof(TextStyleLevel));
_COM_SMARTPTR_TYPEDEF(TextStyleLevels, __uuidof(TextStyleLevels));
_COM_SMARTPTR_TYPEDEF(Borders, __uuidof(Borders));
_COM_SMARTPTR_TYPEDEF(Pane, __uuidof(Pane));
_COM_SMARTPTR_TYPEDEF(Panes, __uuidof(Panes));
_COM_SMARTPTR_TYPEDEF(DefaultWebOptions, __uuidof(DefaultWebOptions));
_COM_SMARTPTR_TYPEDEF(WebOptions, __uuidof(WebOptions));
_COM_SMARTPTR_TYPEDEF(PublishObject, __uuidof(PublishObject));
_COM_SMARTPTR_TYPEDEF(PublishObjects, __uuidof(PublishObjects));
_COM_SMARTPTR_TYPEDEF(MasterEvents, __uuidof(MasterEvents));
_COM_SMARTPTR_TYPEDEF(TextRange, __uuidof(TextRange));
_COM_SMARTPTR_TYPEDEF(TextFrame, __uuidof(TextFrame));
_COM_SMARTPTR_TYPEDEF(TextStyle, __uuidof(TextStyle));
_COM_SMARTPTR_TYPEDEF(TextStyles, __uuidof(TextStyles));
_COM_SMARTPTR_TYPEDEF(_PowerRex, __uuidof(_PowerRex));
_COM_SMARTPTR_TYPEDEF(Comment, __uuidof(Comment));
_COM_SMARTPTR_TYPEDEF(Comments, __uuidof(Comments));
_COM_SMARTPTR_TYPEDEF(EffectParameters, __uuidof(EffectParameters));
_COM_SMARTPTR_TYPEDEF(EffectInformation, __uuidof(EffectInformation));
_COM_SMARTPTR_TYPEDEF(MotionEffect, __uuidof(MotionEffect));
_COM_SMARTPTR_TYPEDEF(ColorEffect, __uuidof(ColorEffect));
_COM_SMARTPTR_TYPEDEF(ScaleEffect, __uuidof(ScaleEffect));
_COM_SMARTPTR_TYPEDEF(RotationEffect, __uuidof(RotationEffect));
_COM_SMARTPTR_TYPEDEF(AnimationPoint, __uuidof(AnimationPoint));
_COM_SMARTPTR_TYPEDEF(AnimationPoints, __uuidof(AnimationPoints));
_COM_SMARTPTR_TYPEDEF(PropertyEffect, __uuidof(PropertyEffect));
_COM_SMARTPTR_TYPEDEF(AutoCorrect, __uuidof(AutoCorrect));
_COM_SMARTPTR_TYPEDEF(Options, __uuidof(Options));
_COM_SMARTPTR_TYPEDEF(CommandEffect, __uuidof(CommandEffect));
_COM_SMARTPTR_TYPEDEF(FilterEffect, __uuidof(FilterEffect));
_COM_SMARTPTR_TYPEDEF(SetEffect, __uuidof(SetEffect));
_COM_SMARTPTR_TYPEDEF(TableStyle, __uuidof(TableStyle));
_COM_SMARTPTR_TYPEDEF(CustomerData, __uuidof(CustomerData));
_COM_SMARTPTR_TYPEDEF(Research, __uuidof(Research));
_COM_SMARTPTR_TYPEDEF(TableBackground, __uuidof(TableBackground));
_COM_SMARTPTR_TYPEDEF(TextFrame2, __uuidof(TextFrame2));
_COM_SMARTPTR_TYPEDEF(ChartBorder, __uuidof(ChartBorder));
_COM_SMARTPTR_TYPEDEF(ChartColorFormat, __uuidof(ChartColorFormat));
_COM_SMARTPTR_TYPEDEF(ChartData, __uuidof(ChartData));
_COM_SMARTPTR_TYPEDEF(ChartFillFormat, __uuidof(ChartFillFormat));
_COM_SMARTPTR_TYPEDEF(ChartFormat, __uuidof(ChartFormat));
_COM_SMARTPTR_TYPEDEF(Corners, __uuidof(Corners));
_COM_SMARTPTR_TYPEDEF(DropLines, __uuidof(DropLines));
_COM_SMARTPTR_TYPEDEF(ErrorBars, __uuidof(ErrorBars));
_COM_SMARTPTR_TYPEDEF(ChartFont, __uuidof(ChartFont));
_COM_SMARTPTR_TYPEDEF(ChartCharacters, __uuidof(ChartCharacters));
_COM_SMARTPTR_TYPEDEF(DataTable, __uuidof(DataTable));
_COM_SMARTPTR_TYPEDEF(Gridlines, __uuidof(Gridlines));
_COM_SMARTPTR_TYPEDEF(HiLoLines, __uuidof(HiLoLines));
_COM_SMARTPTR_TYPEDEF(Interior, __uuidof(Interior));
_COM_SMARTPTR_TYPEDEF(AxisTitle, __uuidof(AxisTitle));
_COM_SMARTPTR_TYPEDEF(ChartArea, __uuidof(ChartArea));
_COM_SMARTPTR_TYPEDEF(ChartTitle, __uuidof(ChartTitle));
_COM_SMARTPTR_TYPEDEF(DataLabel, __uuidof(DataLabel));
_COM_SMARTPTR_TYPEDEF(DataLabels, __uuidof(DataLabels));
_COM_SMARTPTR_TYPEDEF(DisplayUnitLabel, __uuidof(DisplayUnitLabel));
_COM_SMARTPTR_TYPEDEF(DownBars, __uuidof(DownBars));
_COM_SMARTPTR_TYPEDEF(Floor, __uuidof(Floor));
_COM_SMARTPTR_TYPEDEF(LeaderLines, __uuidof(LeaderLines));
_COM_SMARTPTR_TYPEDEF(Legend, __uuidof(Legend));
_COM_SMARTPTR_TYPEDEF(LegendKey, __uuidof(LegendKey));
_COM_SMARTPTR_TYPEDEF(LegendEntry, __uuidof(LegendEntry));
_COM_SMARTPTR_TYPEDEF(LegendEntries, __uuidof(LegendEntries));
_COM_SMARTPTR_TYPEDEF(PlotArea, __uuidof(PlotArea));
_COM_SMARTPTR_TYPEDEF(Series, __uuidof(Series));
_COM_SMARTPTR_TYPEDEF(SeriesCollection, __uuidof(SeriesCollection));
_COM_SMARTPTR_TYPEDEF(SeriesLines, __uuidof(SeriesLines));
_COM_SMARTPTR_TYPEDEF(TickLabels, __uuidof(TickLabels));
_COM_SMARTPTR_TYPEDEF(Axis, __uuidof(Axis));
_COM_SMARTPTR_TYPEDEF(Axes, __uuidof(Axes));
_COM_SMARTPTR_TYPEDEF(Trendline, __uuidof(Trendline));
_COM_SMARTPTR_TYPEDEF(Trendlines, __uuidof(Trendlines));
_COM_SMARTPTR_TYPEDEF(UpBars, __uuidof(UpBars));
_COM_SMARTPTR_TYPEDEF(ChartGroup, __uuidof(ChartGroup));
_COM_SMARTPTR_TYPEDEF(ChartGroups, __uuidof(ChartGroups));
_COM_SMARTPTR_TYPEDEF(Walls, __uuidof(Walls));
_COM_SMARTPTR_TYPEDEF(SectionProperties, __uuidof(SectionProperties));
_COM_SMARTPTR_TYPEDEF(Player, __uuidof(Player));
_COM_SMARTPTR_TYPEDEF(View, __uuidof(View));
_COM_SMARTPTR_TYPEDEF(SlideShowView, __uuidof(SlideShowView));
_COM_SMARTPTR_TYPEDEF(SlideShowWindow, __uuidof(SlideShowWindow));
_COM_SMARTPTR_TYPEDEF(SlideShowWindows, __uuidof(SlideShowWindows));
_COM_SMARTPTR_TYPEDEF(SlideShowSettings, __uuidof(SlideShowSettings));
_COM_SMARTPTR_TYPEDEF(MediaBookmark, __uuidof(MediaBookmark));
_COM_SMARTPTR_TYPEDEF(MediaBookmarks, __uuidof(MediaBookmarks));
_COM_SMARTPTR_TYPEDEF(MediaFormat, __uuidof(MediaFormat));
_COM_SMARTPTR_TYPEDEF(Coauthoring, __uuidof(Coauthoring));
_COM_SMARTPTR_TYPEDEF(Broadcast, __uuidof(Broadcast));
_COM_SMARTPTR_TYPEDEF(Point, __uuidof(Point));
_COM_SMARTPTR_TYPEDEF(Points, __uuidof(Points));
_COM_SMARTPTR_TYPEDEF(ProtectedViewWindow, __uuidof(ProtectedViewWindow));
_COM_SMARTPTR_TYPEDEF(ProtectedViewWindows, __uuidof(ProtectedViewWindows));
_COM_SMARTPTR_TYPEDEF(_Application, __uuidof(_Application));
_COM_SMARTPTR_TYPEDEF(_Global, __uuidof(_Global));
_COM_SMARTPTR_TYPEDEF(Selection, __uuidof(Selection));
_COM_SMARTPTR_TYPEDEF(DocumentWindow, __uuidof(DocumentWindow));
_COM_SMARTPTR_TYPEDEF(DocumentWindows, __uuidof(DocumentWindows));
_COM_SMARTPTR_TYPEDEF(Slides, __uuidof(Slides));
_COM_SMARTPTR_TYPEDEF(_Slide, __uuidof(_Slide));
_COM_SMARTPTR_TYPEDEF(SlideRange, __uuidof(SlideRange));
_COM_SMARTPTR_TYPEDEF(_Master, __uuidof(_Master));
_COM_SMARTPTR_TYPEDEF(Design, __uuidof(Design));
_COM_SMARTPTR_TYPEDEF(Designs, __uuidof(Designs));
_COM_SMARTPTR_TYPEDEF(Shapes, __uuidof(Shapes));
_COM_SMARTPTR_TYPEDEF(Chart, __uuidof(Chart));
_COM_SMARTPTR_TYPEDEF(Placeholders, __uuidof(Placeholders));
_COM_SMARTPTR_TYPEDEF(FreeformBuilder, __uuidof(FreeformBuilder));
_COM_SMARTPTR_TYPEDEF(Shape, __uuidof(Shape));
_COM_SMARTPTR_TYPEDEF(ConnectorFormat, __uuidof(ConnectorFormat));
_COM_SMARTPTR_TYPEDEF(_Presentation, __uuidof(_Presentation));
_COM_SMARTPTR_TYPEDEF(Cell, __uuidof(Cell));
_COM_SMARTPTR_TYPEDEF(CellRange, __uuidof(CellRange));
_COM_SMARTPTR_TYPEDEF(Column, __uuidof(Column));
_COM_SMARTPTR_TYPEDEF(Columns, __uuidof(Columns));
_COM_SMARTPTR_TYPEDEF(Row, __uuidof(Row));
_COM_SMARTPTR_TYPEDEF(Rows, __uuidof(Rows));
_COM_SMARTPTR_TYPEDEF(Table, __uuidof(Table));
_COM_SMARTPTR_TYPEDEF(Timing, __uuidof(Timing));
_COM_SMARTPTR_TYPEDEF(AnimationBehavior, __uuidof(AnimationBehavior));
_COM_SMARTPTR_TYPEDEF(AnimationBehaviors, __uuidof(AnimationBehaviors));
_COM_SMARTPTR_TYPEDEF(Effect, __uuidof(Effect));
_COM_SMARTPTR_TYPEDEF(EApplication, __uuidof(EApplication));
_COM_SMARTPTR_TYPEDEF(Sequence, __uuidof(Sequence));
_COM_SMARTPTR_TYPEDEF(Sequences, __uuidof(Sequences));
_COM_SMARTPTR_TYPEDEF(TimeLine, __uuidof(TimeLine));
_COM_SMARTPTR_TYPEDEF(ResampleMediaTask, __uuidof(ResampleMediaTask));
_COM_SMARTPTR_TYPEDEF(ResampleMediaTasks, __uuidof(ResampleMediaTasks));
_COM_SMARTPTR_TYPEDEF(ShapeRange, __uuidof(ShapeRange));
_COM_SMARTPTR_TYPEDEF(GroupShapes, __uuidof(GroupShapes));
_COM_SMARTPTR_TYPEDEF(CanvasShapes, __uuidof(CanvasShapes));
_COM_SMARTPTR_TYPEDEF(CustomLayout, __uuidof(CustomLayout));
_COM_SMARTPTR_TYPEDEF(CustomLayouts, __uuidof(CustomLayouts));
_COM_SMARTPTR_TYPEDEF(DiagramNode, __uuidof(DiagramNode));
_COM_SMARTPTR_TYPEDEF(DiagramNodeChildren, __uuidof(DiagramNodeChildren));
_COM_SMARTPTR_TYPEDEF(DiagramNodes, __uuidof(DiagramNodes));
_COM_SMARTPTR_TYPEDEF(Diagram, __uuidof(Diagram));
_COM_SMARTPTR_TYPEDEF(FileConverters, __uuidof(FileConverters));
_COM_SMARTPTR_TYPEDEF(FileConverter, __uuidof(FileConverter));

//
// Type library items
//

enum __declspec(uuid("b2cd8e94-209b-4fb7-8fa1-d3f682efbeda"))
PpWindowState
{
    ppWindowNormal = 1,
    ppWindowMinimized = 2,
    ppWindowMaximized = 3
};

enum __declspec(uuid("9fb6d0e7-a063-4a3a-9e65-4ba1c830f0f7"))
PpArrangeStyle
{
    ppArrangeTiled = 1,
    ppArrangeCascade = 2
};

enum __declspec(uuid("b57d2996-78b9-4f86-a0d3-efd3a3896175"))
PpViewType
{
    ppViewSlide = 1,
    ppViewSlideMaster = 2,
    ppViewNotesPage = 3,
    ppViewHandoutMaster = 4,
    ppViewNotesMaster = 5,
    ppViewOutline = 6,
    ppViewSlideSorter = 7,
    ppViewTitleMaster = 8,
    ppViewNormal = 9,
    ppViewPrintPreview = 10,
    ppViewThumbnails = 11,
    ppViewMasterThumbnails = 12
};

enum __declspec(uuid("6d5a5494-54c4-4e15-90d3-62e5aaad2605"))
PpColorSchemeIndex
{
    ppSchemeColorMixed = -2,
    ppNotSchemeColor = 0,
    ppBackground = 1,
    ppForeground = 2,
    ppShadow = 3,
    ppTitle = 4,
    ppFill = 5,
    ppAccent1 = 6,
    ppAccent2 = 7,
    ppAccent3 = 8
};

enum __declspec(uuid("3724e368-dd02-4f14-a8b5-ff234a727ccc"))
PpSlideSizeType
{
    ppSlideSizeOnScreen = 1,
    ppSlideSizeLetterPaper = 2,
    ppSlideSizeA4Paper = 3,
    ppSlideSize35MM = 4,
    ppSlideSizeOverhead = 5,
    ppSlideSizeBanner = 6,
    ppSlideSizeCustom = 7,
    ppSlideSizeLedgerPaper = 8,
    ppSlideSizeA3Paper = 9,
    ppSlideSizeB4ISOPaper = 10,
    ppSlideSizeB5ISOPaper = 11,
    ppSlideSizeB4JISPaper = 12,
    ppSlideSizeB5JISPaper = 13,
    ppSlideSizeHagakiCard = 14,
    ppSlideSizeOnScreen16x9 = 15,
    ppSlideSizeOnScreen16x10 = 16
};

enum __declspec(uuid("01f8f37d-78d4-4920-b2a2-227b23a7ed66"))
PpSaveAsFileType
{
    ppSaveAsPresentation = 1,
    ppSaveAsPowerPoint7 = 2,
    ppSaveAsPowerPoint4 = 3,
    ppSaveAsPowerPoint3 = 4,
    ppSaveAsTemplate = 5,
    ppSaveAsRTF = 6,
    ppSaveAsShow = 7,
    ppSaveAsAddIn = 8,
    ppSaveAsPowerPoint4FarEast = 10,
    ppSaveAsDefault = 11,
    ppSaveAsHTML = 12,
    ppSaveAsHTMLv3 = 13,
    ppSaveAsHTMLDual = 14,
    ppSaveAsMetaFile = 15,
    ppSaveAsGIF = 16,
    ppSaveAsJPG = 17,
    ppSaveAsPNG = 18,
    ppSaveAsBMP = 19,
    ppSaveAsWebArchive = 20,
    ppSaveAsTIF = 21,
    ppSaveAsPresForReview = 22,
    ppSaveAsEMF = 23,
    ppSaveAsOpenXMLPresentation = 24,
    ppSaveAsOpenXMLPresentationMacroEnabled = 25,
    ppSaveAsOpenXMLTemplate = 26,
    ppSaveAsOpenXMLTemplateMacroEnabled = 27,
    ppSaveAsOpenXMLShow = 28,
    ppSaveAsOpenXMLShowMacroEnabled = 29,
    ppSaveAsOpenXMLAddin = 30,
    ppSaveAsOpenXMLTheme = 31,
    ppSaveAsPDF = 32,
    ppSaveAsXPS = 33,
    ppSaveAsXMLPresentation = 34,
    ppSaveAsOpenDocumentPresentation = 35,
    ppSaveAsOpenXMLPicturePresentation = 36,
    ppSaveAsWMV = 37,
    ppSaveAsExternalConverter = 64000
};

enum __declspec(uuid("f7f8452d-30c2-4866-87bf-455a9fd787d8"))
PpTextStyleType
{
    ppDefaultStyle = 1,
    ppTitleStyle = 2,
    ppBodyStyle = 3
};

enum __declspec(uuid("2d20e1ea-0942-4429-8725-916ea0d91045"))
PpSlideLayout
{
    ppLayoutMixed = -2,
    ppLayoutTitle = 1,
    ppLayoutText = 2,
    ppLayoutTwoColumnText = 3,
    ppLayoutTable = 4,
    ppLayoutTextAndChart = 5,
    ppLayoutChartAndText = 6,
    ppLayoutOrgchart = 7,
    ppLayoutChart = 8,
    ppLayoutTextAndClipart = 9,
    ppLayoutClipartAndText = 10,
    ppLayoutTitleOnly = 11,
    ppLayoutBlank = 12,
    ppLayoutTextAndObject = 13,
    ppLayoutObjectAndText = 14,
    ppLayoutLargeObject = 15,
    ppLayoutObject = 16,
    ppLayoutTextAndMediaClip = 17,
    ppLayoutMediaClipAndText = 18,
    ppLayoutObjectOverText = 19,
    ppLayoutTextOverObject = 20,
    ppLayoutTextAndTwoObjects = 21,
    ppLayoutTwoObjectsAndText = 22,
    ppLayoutTwoObjectsOverText = 23,
    ppLayoutFourObjects = 24,
    ppLayoutVerticalText = 25,
    ppLayoutClipArtAndVerticalText = 26,
    ppLayoutVerticalTitleAndText = 27,
    ppLayoutVerticalTitleAndTextOverChart = 28,
    ppLayoutTwoObjects = 29,
    ppLayoutObjectAndTwoObjects = 30,
    ppLayoutTwoObjectsAndObject = 31,
    ppLayoutCustom = 32,
    ppLayoutSectionHeader = 33,
    ppLayoutComparison = 34,
    ppLayoutContentWithCaption = 35,
    ppLayoutPictureWithCaption = 36
};

enum __declspec(uuid("b5a60d8c-605c-4784-ba39-fb4b9aaeea01"))
PpEntryEffect
{
    ppEffectMixed = -2,
    ppEffectNone = 0,
    ppEffectCut = 257,
    ppEffectCutThroughBlack = 258,
    ppEffectRandom = 513,
    ppEffectBlindsHorizontal = 769,
    ppEffectBlindsVertical = 770,
    ppEffectCheckerboardAcross = 1025,
    ppEffectCheckerboardDown = 1026,
    ppEffectCoverLeft = 1281,
    ppEffectCoverUp = 1282,
    ppEffectCoverRight = 1283,
    ppEffectCoverDown = 1284,
    ppEffectCoverLeftUp = 1285,
    ppEffectCoverRightUp = 1286,
    ppEffectCoverLeftDown = 1287,
    ppEffectCoverRightDown = 1288,
    ppEffectDissolve = 1537,
    ppEffectFade = 1793,
    ppEffectUncoverLeft = 2049,
    ppEffectUncoverUp = 2050,
    ppEffectUncoverRight = 2051,
    ppEffectUncoverDown = 2052,
    ppEffectUncoverLeftUp = 2053,
    ppEffectUncoverRightUp = 2054,
    ppEffectUncoverLeftDown = 2055,
    ppEffectUncoverRightDown = 2056,
    ppEffectRandomBarsHorizontal = 2305,
    ppEffectRandomBarsVertical = 2306,
    ppEffectStripsUpLeft = 2561,
    ppEffectStripsUpRight = 2562,
    ppEffectStripsDownLeft = 2563,
    ppEffectStripsDownRight = 2564,
    ppEffectStripsLeftUp = 2565,
    ppEffectStripsRightUp = 2566,
    ppEffectStripsLeftDown = 2567,
    ppEffectStripsRightDown = 2568,
    ppEffectWipeLeft = 2817,
    ppEffectWipeUp = 2818,
    ppEffectWipeRight = 2819,
    ppEffectWipeDown = 2820,
    ppEffectBoxOut = 3073,
    ppEffectBoxIn = 3074,
    ppEffectFlyFromLeft = 3329,
    ppEffectFlyFromTop = 3330,
    ppEffectFlyFromRight = 3331,
    ppEffectFlyFromBottom = 3332,
    ppEffectFlyFromTopLeft = 3333,
    ppEffectFlyFromTopRight = 3334,
    ppEffectFlyFromBottomLeft = 3335,
    ppEffectFlyFromBottomRight = 3336,
    ppEffectPeekFromLeft = 3337,
    ppEffectPeekFromDown = 3338,
    ppEffectPeekFromRight = 3339,
    ppEffectPeekFromUp = 3340,
    ppEffectCrawlFromLeft = 3341,
    ppEffectCrawlFromUp = 3342,
    ppEffectCrawlFromRight = 3343,
    ppEffectCrawlFromDown = 3344,
    ppEffectZoomIn = 3345,
    ppEffectZoomInSlightly = 3346,
    ppEffectZoomOut = 3347,
    ppEffectZoomOutSlightly = 3348,
    ppEffectZoomCenter = 3349,
    ppEffectZoomBottom = 3350,
    ppEffectStretchAcross = 3351,
    ppEffectStretchLeft = 3352,
    ppEffectStretchUp = 3353,
    ppEffectStretchRight = 3354,
    ppEffectStretchDown = 3355,
    ppEffectSwivel = 3356,
    ppEffectSpiral = 3357,
    ppEffectSplitHorizontalOut = 3585,
    ppEffectSplitHorizontalIn = 3586,
    ppEffectSplitVerticalOut = 3587,
    ppEffectSplitVerticalIn = 3588,
    ppEffectFlashOnceFast = 3841,
    ppEffectFlashOnceMedium = 3842,
    ppEffectFlashOnceSlow = 3843,
    ppEffectAppear = 3844,
    ppEffectCircleOut = 3845,
    ppEffectDiamondOut = 3846,
    ppEffectCombHorizontal = 3847,
    ppEffectCombVertical = 3848,
    ppEffectFadeSmoothly = 3849,
    ppEffectNewsflash = 3850,
    ppEffectPlusOut = 3851,
    ppEffectPushDown = 3852,
    ppEffectPushLeft = 3853,
    ppEffectPushRight = 3854,
    ppEffectPushUp = 3855,
    ppEffectWedge = 3856,
    ppEffectWheel1Spoke = 3857,
    ppEffectWheel2Spokes = 3858,
    ppEffectWheel3Spokes = 3859,
    ppEffectWheel4Spokes = 3860,
    ppEffectWheel8Spokes = 3861,
    ppEffectWheelReverse1Spoke = 3862,
    ppEffectVortexLeft = 3863,
    ppEffectVortexUp = 3864,
    ppEffectVortexRight = 3865,
    ppEffectVortexDown = 3866,
    ppEffectRippleCenter = 3867,
    ppEffectRippleRightUp = 3868,
    ppEffectRippleLeftUp = 3869,
    ppEffectRippleLeftDown = 3870,
    ppEffectRippleRightDown = 3871,
    ppEffectGlitterDiamondLeft = 3872,
    ppEffectGlitterDiamondUp = 3873,
    ppEffectGlitterDiamondRight = 3874,
    ppEffectGlitterDiamondDown = 3875,
    ppEffectGlitterHexagonLeft = 3876,
    ppEffectGlitterHexagonUp = 3877,
    ppEffectGlitterHexagonRight = 3878,
    ppEffectGlitterHexagonDown = 3879,
    ppEffectGalleryLeft = 3880,
    ppEffectGalleryRight = 3881,
    ppEffectConveyorLeft = 3882,
    ppEffectConveyorRight = 3883,
    ppEffectDoorsVertical = 3884,
    ppEffectDoorsHorizontal = 3885,
    ppEffectWindowVertical = 3886,
    ppEffectWindowHorizontal = 3887,
    ppEffectWarpIn = 3888,
    ppEffectWarpOut = 3889,
    ppEffectFlyThroughIn = 3890,
    ppEffectFlyThroughOut = 3891,
    ppEffectFlyThroughInBounce = 3892,
    ppEffectFlyThroughOutBounce = 3893,
    ppEffectRevealSmoothLeft = 3894,
    ppEffectRevealSmoothRight = 3895,
    ppEffectRevealBlackLeft = 3896,
    ppEffectRevealBlackRight = 3897,
    ppEffectHoneycomb = 3898,
    ppEffectFerrisWheelLeft = 3899,
    ppEffectFerrisWheelRight = 3900,
    ppEffectSwitchLeft = 3901,
    ppEffectSwitchUp = 3902,
    ppEffectSwitchRight = 3903,
    ppEffectSwitchDown = 3904,
    ppEffectFlipLeft = 3905,
    ppEffectFlipUp = 3906,
    ppEffectFlipRight = 3907,
    ppEffectFlipDown = 3908,
    ppEffectFlashbulb = 3909,
    ppEffectShredStripsIn = 3910,
    ppEffectShredStripsOut = 3911,
    ppEffectShredRectangleIn = 3912,
    ppEffectShredRectangleOut = 3913,
    ppEffectCubeLeft = 3914,
    ppEffectCubeUp = 3915,
    ppEffectCubeRight = 3916,
    ppEffectCubeDown = 3917,
    ppEffectRotateLeft = 3918,
    ppEffectRotateUp = 3919,
    ppEffectRotateRight = 3920,
    ppEffectRotateDown = 3921,
    ppEffectBoxLeft = 3922,
    ppEffectBoxUp = 3923,
    ppEffectBoxRight = 3924,
    ppEffectBoxDown = 3925,
    ppEffectOrbitLeft = 3926,
    ppEffectOrbitUp = 3927,
    ppEffectOrbitRight = 3928,
    ppEffectOrbitDown = 3929,
    ppEffectPanLeft = 3930,
    ppEffectPanUp = 3931,
    ppEffectPanRight = 3932,
    ppEffectPanDown = 3933
};

enum __declspec(uuid("efcd481c-b00b-4718-a6ff-51d98493ee88"))
PpTextLevelEffect
{
    ppAnimateLevelMixed = -2,
    ppAnimateLevelNone = 0,
    ppAnimateByFirstLevel = 1,
    ppAnimateBySecondLevel = 2,
    ppAnimateByThirdLevel = 3,
    ppAnimateByFourthLevel = 4,
    ppAnimateByFifthLevel = 5,
    ppAnimateByAllLevels = 16
};

enum __declspec(uuid("9688fa96-8cf6-4642-8839-dd659d86c9b9"))
PpTextUnitEffect
{
    ppAnimateUnitMixed = -2,
    ppAnimateByParagraph = 0,
    ppAnimateByWord = 1,
    ppAnimateByCharacter = 2
};

enum __declspec(uuid("b46f41b2-4241-49d4-b772-6182b7f4398d"))
PpChartUnitEffect
{
    ppAnimateChartMixed = -2,
    ppAnimateBySeries = 1,
    ppAnimateByCategory = 2,
    ppAnimateBySeriesElements = 3,
    ppAnimateByCategoryElements = 4,
    ppAnimateChartAllAtOnce = 5
};

enum __declspec(uuid("4e58b80c-d41e-470a-a2f8-05373ca3ea5d"))
PpAfterEffect
{
    ppAfterEffectMixed = -2,
    ppAfterEffectNothing = 0,
    ppAfterEffectHide = 1,
    ppAfterEffectDim = 2,
    ppAfterEffectHideOnClick = 3
};

enum __declspec(uuid("f60170d6-43ca-47a4-88bf-f782728e1c87"))
PpAdvanceMode
{
    ppAdvanceModeMixed = -2,
    ppAdvanceOnClick = 1,
    ppAdvanceOnTime = 2
};

enum __declspec(uuid("eaf0356f-6b2b-4163-a40f-4dd7d6b40a43"))
PpSoundEffectType
{
    ppSoundEffectsMixed = -2,
    ppSoundNone = 0,
    ppSoundStopPrevious = 1,
    ppSoundFile = 2
};

enum __declspec(uuid("927d5c5e-9e3b-46e0-ae7b-3e89a0b53ff7"))
PpFollowColors
{
    ppFollowColorsMixed = -2,
    ppFollowColorsNone = 0,
    ppFollowColorsScheme = 1,
    ppFollowColorsTextAndBackground = 2
};

enum __declspec(uuid("939abc08-6f0e-4595-a12f-f96cfcb7ff77"))
PpUpdateOption
{
    ppUpdateOptionMixed = -2,
    ppUpdateOptionManual = 1,
    ppUpdateOptionAutomatic = 2
};

enum __declspec(uuid("07012de4-763e-467d-8b87-1e10304332f8"))
PpParagraphAlignment
{
    ppAlignmentMixed = -2,
    ppAlignLeft = 1,
    ppAlignCenter = 2,
    ppAlignRight = 3,
    ppAlignJustify = 4,
    ppAlignDistribute = 5,
    ppAlignThaiDistribute = 6,
    ppAlignJustifyLow = 7
};

enum __declspec(uuid("28f0103e-1d08-431e-8cdd-5554d008bbc3"))
PpBaselineAlignment
{
    ppBaselineAlignMixed = -2,
    ppBaselineAlignBaseline = 1,
    ppBaselineAlignTop = 2,
    ppBaselineAlignCenter = 3,
    ppBaselineAlignFarEast50 = 4,
    ppBaselineAlignAuto = 5
};

enum __declspec(uuid("f08dd520-1e7a-4812-9ce2-a96b079299d1"))
PpTabStopType
{
    ppTabStopMixed = -2,
    ppTabStopLeft = 1,
    ppTabStopCenter = 2,
    ppTabStopRight = 3,
    ppTabStopDecimal = 4
};

enum __declspec(uuid("cc8f164e-ebd7-4366-8dd6-76e984c2a8f5"))
PpIndentControl
{
    ppIndentControlMixed = -2,
    ppIndentReplaceAttr = 1,
    ppIndentKeepAttr = 2
};

enum __declspec(uuid("57187984-0cfa-4ad7-955a-ada220110fc2"))
PpChangeCase
{
    ppCaseSentence = 1,
    ppCaseLower = 2,
    ppCaseUpper = 3,
    ppCaseTitle = 4,
    ppCaseToggle = 5
};

enum __declspec(uuid("a14b5d3e-eaa8-45c0-a41c-d0aa208ba279"))
PpSlideShowPointerType
{
    ppSlideShowPointerNone = 0,
    ppSlideShowPointerArrow = 1,
    ppSlideShowPointerPen = 2,
    ppSlideShowPointerAlwaysHidden = 3,
    ppSlideShowPointerAutoArrow = 4,
    ppSlideShowPointerEraser = 5
};

enum __declspec(uuid("fb066093-3dff-4c61-9ae6-6c76c635fd55"))
PpSlideShowState
{
    ppSlideShowRunning = 1,
    ppSlideShowPaused = 2,
    ppSlideShowBlackScreen = 3,
    ppSlideShowWhiteScreen = 4,
    ppSlideShowDone = 5
};

enum __declspec(uuid("98884cd0-f359-48cb-bed8-0ec3090a59d6"))
PpSlideShowAdvanceMode
{
    ppSlideShowManualAdvance = 1,
    ppSlideShowUseSlideTimings = 2,
    ppSlideShowRehearseNewTimings = 3
};

enum __declspec(uuid("6911a8c8-1743-4aaf-988b-55ff9ae94329"))
PpFileDialogType
{
    ppFileDialogOpen = 1,
    ppFileDialogSave = 2
};

enum __declspec(uuid("8d624138-9944-443c-8be1-eb10e90dd8c6"))
PpPrintOutputType
{
    ppPrintOutputSlides = 1,
    ppPrintOutputTwoSlideHandouts = 2,
    ppPrintOutputThreeSlideHandouts = 3,
    ppPrintOutputSixSlideHandouts = 4,
    ppPrintOutputNotesPages = 5,
    ppPrintOutputOutline = 6,
    ppPrintOutputBuildSlides = 7,
    ppPrintOutputFourSlideHandouts = 8,
    ppPrintOutputNineSlideHandouts = 9,
    ppPrintOutputOneSlideHandouts = 10
};

enum __declspec(uuid("d3a902f8-8526-4ddb-a72e-218c989128db"))
PpPrintHandoutOrder
{
    ppPrintHandoutVerticalFirst = 1,
    ppPrintHandoutHorizontalFirst = 2
};

enum __declspec(uuid("901438d1-d781-49ca-aeca-83d6fc958d6a"))
PpPrintColorType
{
    ppPrintColor = 1,
    ppPrintBlackAndWhite = 2,
    ppPrintPureBlackAndWhite = 3
};

enum __declspec(uuid("68cf3b66-38fc-4cc7-88ca-07c394cc6a4a"))
PpSelectionType
{
    ppSelectionNone = 0,
    ppSelectionSlides = 1,
    ppSelectionShapes = 2,
    ppSelectionText = 3
};

enum __declspec(uuid("f37b6599-0312-44bf-984e-1136c06cdab2"))
PpDirection
{
    ppDirectionMixed = -2,
    ppDirectionLeftToRight = 1,
    ppDirectionRightToLeft = 2
};

enum __declspec(uuid("1bea23a4-497f-44d7-b7d8-80eb71a02fb5"))
PpDateTimeFormat
{
    ppDateTimeFormatMixed = -2,
    ppDateTimeMdyy = 1,
    ppDateTimeddddMMMMddyyyy = 2,
    ppDateTimedMMMMyyyy = 3,
    ppDateTimeMMMMdyyyy = 4,
    ppDateTimedMMMyy = 5,
    ppDateTimeMMMMyy = 6,
    ppDateTimeMMyy = 7,
    ppDateTimeMMddyyHmm = 8,
    ppDateTimeMMddyyhmmAMPM = 9,
    ppDateTimeHmm = 10,
    ppDateTimeHmmss = 11,
    ppDateTimehmmAMPM = 12,
    ppDateTimehmmssAMPM = 13,
    ppDateTimeFigureOut = 14
};

enum __declspec(uuid("6dc8f1ec-4f59-43d5-97d9-efc09d099444"))
PpTransitionSpeed
{
    ppTransitionSpeedMixed = -2,
    ppTransitionSpeedSlow = 1,
    ppTransitionSpeedMedium = 2,
    ppTransitionSpeedFast = 3
};

enum __declspec(uuid("5106e4a7-532c-413f-b2c7-8d532008f511"))
PpMouseActivation
{
    ppMouseClick = 1,
    ppMouseOver = 2
};

enum __declspec(uuid("eb78ae17-fad3-4edf-8a0b-0a379069749b"))
PpActionType
{
    ppActionMixed = -2,
    ppActionNone = 0,
    ppActionNextSlide = 1,
    ppActionPreviousSlide = 2,
    ppActionFirstSlide = 3,
    ppActionLastSlide = 4,
    ppActionLastSlideViewed = 5,
    ppActionEndShow = 6,
    ppActionHyperlink = 7,
    ppActionRunMacro = 8,
    ppActionRunProgram = 9,
    ppActionNamedSlideShow = 10,
    ppActionOLEVerb = 11,
    ppActionPlay = 12
};

enum __declspec(uuid("adeadb7e-f268-4574-90fe-bc0bf4b28b3c"))
PpPlaceholderType
{
    ppPlaceholderMixed = -2,
    ppPlaceholderTitle = 1,
    ppPlaceholderBody = 2,
    ppPlaceholderCenterTitle = 3,
    ppPlaceholderSubtitle = 4,
    ppPlaceholderVerticalTitle = 5,
    ppPlaceholderVerticalBody = 6,
    ppPlaceholderObject = 7,
    ppPlaceholderChart = 8,
    ppPlaceholderBitmap = 9,
    ppPlaceholderMediaClip = 10,
    ppPlaceholderOrgChart = 11,
    ppPlaceholderTable = 12,
    ppPlaceholderSlideNumber = 13,
    ppPlaceholderHeader = 14,
    ppPlaceholderFooter = 15,
    ppPlaceholderDate = 16,
    ppPlaceholderVerticalObject = 17,
    ppPlaceholderPicture = 18
};

enum __declspec(uuid("e893bd55-3fc7-3cda-9281-1acb65441c8b"))
PpSlideShowType
{
    ppShowTypeSpeaker = 1,
    ppShowTypeWindow = 2,
    ppShowTypeKiosk = 3,
    ppShowTypeWindow2 = 4
};

enum __declspec(uuid("f8fb4bd4-ece4-4009-9fa6-ac93191315eb"))
PpPrintRangeType
{
    ppPrintAll = 1,
    ppPrintSelection = 2,
    ppPrintCurrent = 3,
    ppPrintSlideRange = 4,
    ppPrintNamedSlideShow = 5,
    ppPrintSection = 6
};

enum __declspec(uuid("a8fb07e5-9c29-4f16-9dc4-ca6333719385"))
PpAutoSize
{
    ppAutoSizeMixed = -2,
    ppAutoSizeNone = 0,
    ppAutoSizeShapeToFitText = 1
};

enum __declspec(uuid("51491ce7-3402-4004-9836-2307103f3731"))
PpMediaType
{
    ppMediaTypeMixed = -2,
    ppMediaTypeOther = 1,
    ppMediaTypeSound = 2,
    ppMediaTypeMovie = 3
};

enum __declspec(uuid("6cb42c6d-2958-4f46-8327-e6abb83f719d"))
PpSoundFormatType
{
    ppSoundFormatMixed = -2,
    ppSoundFormatNone = 0,
    ppSoundFormatWAV = 1,
    ppSoundFormatMIDI = 2,
    ppSoundFormatCDAudio = 3
};

enum __declspec(uuid("c8f6bbd1-4841-4c73-acbb-69ce5637808e"))
PpFarEastLineBreakLevel
{
    ppFarEastLineBreakLevelNormal = 1,
    ppFarEastLineBreakLevelStrict = 2,
    ppFarEastLineBreakLevelCustom = 3
};

enum __declspec(uuid("170ecd11-1508-446d-99f0-a5df077f35ff"))
PpSlideShowRangeType
{
    ppShowAll = 1,
    ppShowSlideRange = 2,
    ppShowNamedSlideShow = 3
};

enum __declspec(uuid("dca36717-111a-4633-b688-32c9548576d4"))
PpFrameColors
{
    ppFrameColorsBrowserColors = 1,
    ppFrameColorsPresentationSchemeTextColor = 2,
    ppFrameColorsPresentationSchemeAccentColor = 3,
    ppFrameColorsWhiteTextOnBlack = 4,
    ppFrameColorsBlackTextOnWhite = 5
};

struct __declspec(uuid("91493450-5a91-11cf-8700-00aa0060263b"))
Collection : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get__NewEnum (
        /*[out,retval]*/ IUnknown * * _NewEnum ) = 0;
      virtual HRESULT __stdcall _Index (
        /*[in]*/ int Index,
        /*[out,retval]*/ VARIANT * _Index ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ long * Count ) = 0;
};

struct __declspec(uuid("91493441-5a91-11cf-8700-00aa0060263b"))
Application;
    // [ default ] interface _Application
    // [ default, source ] interface EApplication

struct __declspec(uuid("91493443-5a91-11cf-8700-00aa0060263b"))
Global;
    // [ default ] interface _Global

struct __declspec(uuid("91493452-5a91-11cf-8700-00aa0060263b"))
ColorFormat : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_RGB (
        /*[out,retval]*/ Office2010::MsoRGBType * RGB ) = 0;
      virtual HRESULT __stdcall put_RGB (
        /*[in]*/ Office2010::MsoRGBType RGB ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum Office2010::MsoColorType * Type ) = 0;
      virtual HRESULT __stdcall get_SchemeColor (
        /*[out,retval]*/ enum PpColorSchemeIndex * SchemeColor ) = 0;
      virtual HRESULT __stdcall put_SchemeColor (
        /*[in]*/ enum PpColorSchemeIndex SchemeColor ) = 0;
      virtual HRESULT __stdcall get_TintAndShade (
        /*[out,retval]*/ float * pValue ) = 0;
      virtual HRESULT __stdcall put_TintAndShade (
        /*[in]*/ float pValue ) = 0;
      virtual HRESULT __stdcall get_ObjectThemeColor (
        /*[out,retval]*/ enum Office2010::MsoThemeColorIndex * ObjectThemeColor ) = 0;
      virtual HRESULT __stdcall put_ObjectThemeColor (
        /*[in]*/ enum Office2010::MsoThemeColorIndex ObjectThemeColor ) = 0;
      virtual HRESULT __stdcall get_Brightness (
        /*[out,retval]*/ float * Brightness ) = 0;
      virtual HRESULT __stdcall put_Brightness (
        /*[in]*/ float Brightness ) = 0;
};

struct __declspec(uuid("9149345c-5a91-11cf-8700-00aa0060263b"))
NamedSlideShow : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
      virtual HRESULT __stdcall get_SlideIDs (
        /*[out,retval]*/ VARIANT * SlideIDs ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ long * Count ) = 0;
};

struct __declspec(uuid("9149345b-5a91-11cf-8700-00aa0060263b"))
NamedSlideShows : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct NamedSlideShow * * Item ) = 0;
      virtual HRESULT __stdcall Add (
        /*[in]*/ BSTR Name,
        /*[in]*/ VARIANT safeArrayOfSlideIDs,
        /*[out,retval]*/ struct NamedSlideShow * * Add ) = 0;
};

struct __declspec(uuid("9149345f-5a91-11cf-8700-00aa0060263b"))
PrintRange : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Start (
        /*[out,retval]*/ int * Start ) = 0;
      virtual HRESULT __stdcall get_End (
        /*[out,retval]*/ int * End ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
};

struct __declspec(uuid("9149345e-5a91-11cf-8700-00aa0060263b"))
PrintRanges : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall Add (
        /*[in]*/ int Start,
        /*[in]*/ int End,
        /*[out,retval]*/ struct PrintRange * * Add ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall ClearAll ( ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct PrintRange * * Item ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
};

struct __declspec(uuid("9149345d-5a91-11cf-8700-00aa0060263b"))
PrintOptions : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_PrintColorType (
        /*[out,retval]*/ enum PpPrintColorType * PrintColorType ) = 0;
      virtual HRESULT __stdcall put_PrintColorType (
        /*[in]*/ enum PpPrintColorType PrintColorType ) = 0;
      virtual HRESULT __stdcall get_Collate (
        /*[out,retval]*/ enum Office2010::MsoTriState * Collate ) = 0;
      virtual HRESULT __stdcall put_Collate (
        /*[in]*/ enum Office2010::MsoTriState Collate ) = 0;
      virtual HRESULT __stdcall get_FitToPage (
        /*[out,retval]*/ enum Office2010::MsoTriState * FitToPage ) = 0;
      virtual HRESULT __stdcall put_FitToPage (
        /*[in]*/ enum Office2010::MsoTriState FitToPage ) = 0;
      virtual HRESULT __stdcall get_FrameSlides (
        /*[out,retval]*/ enum Office2010::MsoTriState * FrameSlides ) = 0;
      virtual HRESULT __stdcall put_FrameSlides (
        /*[in]*/ enum Office2010::MsoTriState FrameSlides ) = 0;
      virtual HRESULT __stdcall get_NumberOfCopies (
        /*[out,retval]*/ int * NumberOfCopies ) = 0;
      virtual HRESULT __stdcall put_NumberOfCopies (
        /*[in]*/ int NumberOfCopies ) = 0;
      virtual HRESULT __stdcall get_OutputType (
        /*[out,retval]*/ enum PpPrintOutputType * OutputType ) = 0;
      virtual HRESULT __stdcall put_OutputType (
        /*[in]*/ enum PpPrintOutputType OutputType ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_PrintHiddenSlides (
        /*[out,retval]*/ enum Office2010::MsoTriState * PrintHiddenSlides ) = 0;
      virtual HRESULT __stdcall put_PrintHiddenSlides (
        /*[in]*/ enum Office2010::MsoTriState PrintHiddenSlides ) = 0;
      virtual HRESULT __stdcall get_PrintInBackground (
        /*[out,retval]*/ enum Office2010::MsoTriState * PrintInBackground ) = 0;
      virtual HRESULT __stdcall put_PrintInBackground (
        /*[in]*/ enum Office2010::MsoTriState PrintInBackground ) = 0;
      virtual HRESULT __stdcall get_RangeType (
        /*[out,retval]*/ enum PpPrintRangeType * RangeType ) = 0;
      virtual HRESULT __stdcall put_RangeType (
        /*[in]*/ enum PpPrintRangeType RangeType ) = 0;
      virtual HRESULT __stdcall get_Ranges (
        /*[out,retval]*/ struct PrintRanges * * Ranges ) = 0;
      virtual HRESULT __stdcall get_PrintFontsAsGraphics (
        /*[out,retval]*/ enum Office2010::MsoTriState * PrintFontsAsGraphics ) = 0;
      virtual HRESULT __stdcall put_PrintFontsAsGraphics (
        /*[in]*/ enum Office2010::MsoTriState PrintFontsAsGraphics ) = 0;
      virtual HRESULT __stdcall get_SlideShowName (
        /*[out,retval]*/ BSTR * SlideShowName ) = 0;
      virtual HRESULT __stdcall put_SlideShowName (
        /*[in]*/ BSTR SlideShowName ) = 0;
      virtual HRESULT __stdcall get_ActivePrinter (
        /*[out,retval]*/ BSTR * ActivePrinter ) = 0;
      virtual HRESULT __stdcall put_ActivePrinter (
        /*[in]*/ BSTR ActivePrinter ) = 0;
      virtual HRESULT __stdcall get_HandoutOrder (
        /*[out,retval]*/ enum PpPrintHandoutOrder * HandoutOrder ) = 0;
      virtual HRESULT __stdcall put_HandoutOrder (
        /*[in]*/ enum PpPrintHandoutOrder HandoutOrder ) = 0;
      virtual HRESULT __stdcall get_PrintComments (
        /*[out,retval]*/ enum Office2010::MsoTriState * PrintComments ) = 0;
      virtual HRESULT __stdcall put_PrintComments (
        /*[in]*/ enum Office2010::MsoTriState PrintComments ) = 0;
      virtual HRESULT __stdcall get_sectionIndex (
        /*[out,retval]*/ int * sectionIndex ) = 0;
      virtual HRESULT __stdcall put_sectionIndex (
        /*[in]*/ int sectionIndex ) = 0;
      virtual HRESULT __stdcall get_HighQuality (
        /*[out,retval]*/ enum Office2010::MsoTriState * HighQuality ) = 0;
      virtual HRESULT __stdcall put_HighQuality (
        /*[in]*/ enum Office2010::MsoTriState HighQuality ) = 0;
};

struct __declspec(uuid("91493461-5a91-11cf-8700-00aa0060263b"))
AddIn : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_FullName (
        /*[out,retval]*/ BSTR * FullName ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall get_Path (
        /*[out,retval]*/ BSTR * Path ) = 0;
      virtual HRESULT __stdcall get_Registered (
        /*[out,retval]*/ enum Office2010::MsoTriState * Registered ) = 0;
      virtual HRESULT __stdcall put_Registered (
        /*[in]*/ enum Office2010::MsoTriState Registered ) = 0;
      virtual HRESULT __stdcall get_AutoLoad (
        /*[out,retval]*/ enum Office2010::MsoTriState * AutoLoad ) = 0;
      virtual HRESULT __stdcall put_AutoLoad (
        /*[in]*/ enum Office2010::MsoTriState AutoLoad ) = 0;
      virtual HRESULT __stdcall get_Loaded (
        /*[out,retval]*/ enum Office2010::MsoTriState * Loaded ) = 0;
      virtual HRESULT __stdcall put_Loaded (
        /*[in]*/ enum Office2010::MsoTriState Loaded ) = 0;
      virtual HRESULT __stdcall get_DisplayAlerts (
        /*[out,retval]*/ enum Office2010::MsoTriState * DisplayAlerts ) = 0;
      virtual HRESULT __stdcall put_DisplayAlerts (
        /*[in]*/ enum Office2010::MsoTriState DisplayAlerts ) = 0;
      virtual HRESULT __stdcall get_RegisteredInHKLM (
        /*[out,retval]*/ enum Office2010::MsoTriState * RegisteredInHKLM ) = 0;
};

struct __declspec(uuid("91493460-5a91-11cf-8700-00aa0060263b"))
AddIns : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT * Index,
        /*[out,retval]*/ struct AddIn * * Item ) = 0;
      virtual HRESULT __stdcall Add (
        /*[in]*/ BSTR FileName,
        /*[out,retval]*/ struct AddIn * * Add ) = 0;
      virtual HRESULT __stdcall Remove (
        /*[in]*/ VARIANT * Index ) = 0;
};

struct __declspec(uuid("91493463-5a91-11cf-8700-00aa0060263b"))
PresEvents : IUnknown
{};

struct __declspec(uuid("91493444-5a91-11cf-8700-00aa0060263b"))
Presentation;
    // [ default ] interface _Presentation
    // [ default, source ] interface PresEvents

struct __declspec(uuid("91493462-5a91-11cf-8700-00aa0060263b"))
Presentations : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct _Presentation * * Item ) = 0;
      virtual HRESULT __stdcall Add (
        /*[in]*/ enum Office2010::MsoTriState WithWindow,
        /*[out,retval]*/ struct _Presentation * * Add ) = 0;
      virtual HRESULT __stdcall OpenOld (
        /*[in]*/ BSTR FileName,
        /*[in]*/ enum Office2010::MsoTriState ReadOnly,
        /*[in]*/ enum Office2010::MsoTriState Untitled,
        /*[in]*/ enum Office2010::MsoTriState WithWindow,
        /*[out,retval]*/ struct _Presentation * * OpenOld ) = 0;
      virtual HRESULT __stdcall Open (
        /*[in]*/ BSTR FileName,
        /*[in]*/ enum Office2010::MsoTriState ReadOnly,
        /*[in]*/ enum Office2010::MsoTriState Untitled,
        /*[in]*/ enum Office2010::MsoTriState WithWindow,
        /*[out,retval]*/ struct _Presentation * * Open ) = 0;
      virtual HRESULT __stdcall CheckOut (
        /*[in]*/ BSTR FileName ) = 0;
      virtual HRESULT __stdcall CanCheckOut (
        /*[in]*/ BSTR FileName,
        /*[out,retval]*/ VARIANT_BOOL * CanCheckOut ) = 0;
      virtual HRESULT __stdcall Open2007 (
        /*[in]*/ BSTR FileName,
        /*[in]*/ enum Office2010::MsoTriState ReadOnly,
        /*[in]*/ enum Office2010::MsoTriState Untitled,
        /*[in]*/ enum Office2010::MsoTriState WithWindow,
        /*[in]*/ enum Office2010::MsoTriState OpenAndRepair,
        /*[out,retval]*/ struct _Presentation * * Open2007 ) = 0;
};

struct __declspec(uuid("91493465-5a91-11cf-8700-00aa0060263b"))
Hyperlink : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum Office2010::MsoHyperlinkType * Type ) = 0;
      virtual HRESULT __stdcall get_Address (
        /*[out,retval]*/ BSTR * Address ) = 0;
      virtual HRESULT __stdcall put_Address (
        /*[in]*/ BSTR Address ) = 0;
      virtual HRESULT __stdcall get_SubAddress (
        /*[out,retval]*/ BSTR * SubAddress ) = 0;
      virtual HRESULT __stdcall put_SubAddress (
        /*[in]*/ BSTR SubAddress ) = 0;
      virtual HRESULT __stdcall AddToFavorites ( ) = 0;
      virtual HRESULT __stdcall get_EmailSubject (
        /*[out,retval]*/ BSTR * EmailSubject ) = 0;
      virtual HRESULT __stdcall put_EmailSubject (
        /*[in]*/ BSTR EmailSubject ) = 0;
      virtual HRESULT __stdcall get_ScreenTip (
        /*[out,retval]*/ BSTR * ScreenTip ) = 0;
      virtual HRESULT __stdcall put_ScreenTip (
        /*[in]*/ BSTR ScreenTip ) = 0;
      virtual HRESULT __stdcall get_TextToDisplay (
        /*[out,retval]*/ BSTR * TextToDisplay ) = 0;
      virtual HRESULT __stdcall put_TextToDisplay (
        /*[in]*/ BSTR TextToDisplay ) = 0;
      virtual HRESULT __stdcall get_ShowAndReturn (
        /*[out,retval]*/ enum Office2010::MsoTriState * ShowAndReturn ) = 0;
      virtual HRESULT __stdcall put_ShowAndReturn (
        /*[in]*/ enum Office2010::MsoTriState ShowAndReturn ) = 0;
      virtual HRESULT __stdcall Follow ( ) = 0;
      virtual HRESULT __stdcall CreateNewDocument (
        /*[in]*/ BSTR FileName,
        /*[in]*/ enum Office2010::MsoTriState EditNow,
        /*[in]*/ enum Office2010::MsoTriState Overwrite ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
};

struct __declspec(uuid("91493464-5a91-11cf-8700-00aa0060263b"))
Hyperlinks : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct Hyperlink * * Item ) = 0;
};

struct __declspec(uuid("91493466-5a91-11cf-8700-00aa0060263b"))
PageSetup : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_FirstSlideNumber (
        /*[out,retval]*/ int * FirstSlideNumber ) = 0;
      virtual HRESULT __stdcall put_FirstSlideNumber (
        /*[in]*/ int FirstSlideNumber ) = 0;
      virtual HRESULT __stdcall get_SlideHeight (
        /*[out,retval]*/ float * SlideHeight ) = 0;
      virtual HRESULT __stdcall put_SlideHeight (
        /*[in]*/ float SlideHeight ) = 0;
      virtual HRESULT __stdcall get_SlideWidth (
        /*[out,retval]*/ float * SlideWidth ) = 0;
      virtual HRESULT __stdcall put_SlideWidth (
        /*[in]*/ float SlideWidth ) = 0;
      virtual HRESULT __stdcall get_SlideSize (
        /*[out,retval]*/ enum PpSlideSizeType * SlideSize ) = 0;
      virtual HRESULT __stdcall put_SlideSize (
        /*[in]*/ enum PpSlideSizeType SlideSize ) = 0;
      virtual HRESULT __stdcall get_NotesOrientation (
        /*[out,retval]*/ enum Office2010::MsoOrientation * NotesOrientation ) = 0;
      virtual HRESULT __stdcall put_NotesOrientation (
        /*[in]*/ enum Office2010::MsoOrientation NotesOrientation ) = 0;
      virtual HRESULT __stdcall get_SlideOrientation (
        /*[out,retval]*/ enum Office2010::MsoOrientation * SlideOrientation ) = 0;
      virtual HRESULT __stdcall put_SlideOrientation (
        /*[in]*/ enum Office2010::MsoOrientation SlideOrientation ) = 0;
};

struct __declspec(uuid("91493468-5a91-11cf-8700-00aa0060263b"))
ExtraColors : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ Office2010::MsoRGBType * Item ) = 0;
      virtual HRESULT __stdcall Add (
        /*[in]*/ Office2010::MsoRGBType Type ) = 0;
      virtual HRESULT __stdcall Clear ( ) = 0;
};

struct __declspec(uuid("9149346d-5a91-11cf-8700-00aa0060263b"))
SldEvents : IUnknown
{};

struct __declspec(uuid("91493445-5a91-11cf-8700-00aa0060263b"))
Slide;
    // [ default ] interface _Slide
    // [ default, source ] interface SldEvents

struct __declspec(uuid("91493470-5a91-11cf-8700-00aa0060263b"))
RGBColor : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_RGB (
        /*[out,retval]*/ Office2010::MsoRGBType * RGB ) = 0;
      virtual HRESULT __stdcall put_RGB (
        /*[in]*/ Office2010::MsoRGBType RGB ) = 0;
};

struct __declspec(uuid("9149346f-5a91-11cf-8700-00aa0060263b"))
ColorScheme : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Colors (
        /*[in]*/ enum PpColorSchemeIndex SchemeColor,
        /*[out,retval]*/ struct RGBColor * * Colors ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
};

struct __declspec(uuid("9149346e-5a91-11cf-8700-00aa0060263b"))
ColorSchemes : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct ColorScheme * * Item ) = 0;
      virtual HRESULT __stdcall Add (
        /*[in]*/ struct ColorScheme * Scheme,
        /*[out,retval]*/ struct ColorScheme * * Add ) = 0;
};

struct __declspec(uuid("91493472-5a91-11cf-8700-00aa0060263b"))
SoundEffect : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall put_Name (
        /*[in]*/ BSTR Name ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum PpSoundEffectType * Type ) = 0;
      virtual HRESULT __stdcall put_Type (
        /*[in]*/ enum PpSoundEffectType Type ) = 0;
      virtual HRESULT __stdcall ImportFromFile (
        /*[in]*/ BSTR FileName ) = 0;
      virtual HRESULT __stdcall Play ( ) = 0;
};

struct __declspec(uuid("91493471-5a91-11cf-8700-00aa0060263b"))
SlideShowTransition : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_AdvanceOnClick (
        /*[out,retval]*/ enum Office2010::MsoTriState * AdvanceOnClick ) = 0;
      virtual HRESULT __stdcall put_AdvanceOnClick (
        /*[in]*/ enum Office2010::MsoTriState AdvanceOnClick ) = 0;
      virtual HRESULT __stdcall get_AdvanceOnTime (
        /*[out,retval]*/ enum Office2010::MsoTriState * AdvanceOnTime ) = 0;
      virtual HRESULT __stdcall put_AdvanceOnTime (
        /*[in]*/ enum Office2010::MsoTriState AdvanceOnTime ) = 0;
      virtual HRESULT __stdcall get_AdvanceTime (
        /*[out,retval]*/ float * AdvanceTime ) = 0;
      virtual HRESULT __stdcall put_AdvanceTime (
        /*[in]*/ float AdvanceTime ) = 0;
      virtual HRESULT __stdcall get_EntryEffect (
        /*[out,retval]*/ enum PpEntryEffect * EntryEffect ) = 0;
      virtual HRESULT __stdcall put_EntryEffect (
        /*[in]*/ enum PpEntryEffect EntryEffect ) = 0;
      virtual HRESULT __stdcall get_Hidden (
        /*[out,retval]*/ enum Office2010::MsoTriState * Hidden ) = 0;
      virtual HRESULT __stdcall put_Hidden (
        /*[in]*/ enum Office2010::MsoTriState Hidden ) = 0;
      virtual HRESULT __stdcall get_LoopSoundUntilNext (
        /*[out,retval]*/ enum Office2010::MsoTriState * LoopSoundUntilNext ) = 0;
      virtual HRESULT __stdcall put_LoopSoundUntilNext (
        /*[in]*/ enum Office2010::MsoTriState LoopSoundUntilNext ) = 0;
      virtual HRESULT __stdcall get_SoundEffect (
        /*[out,retval]*/ struct SoundEffect * * SoundEffect ) = 0;
      virtual HRESULT __stdcall get_Speed (
        /*[out,retval]*/ enum PpTransitionSpeed * Speed ) = 0;
      virtual HRESULT __stdcall put_Speed (
        /*[in]*/ enum PpTransitionSpeed Speed ) = 0;
      virtual HRESULT __stdcall get_Duration (
        /*[out,retval]*/ float * Duration ) = 0;
      virtual HRESULT __stdcall put_Duration (
        /*[in]*/ float Duration ) = 0;
};

struct __declspec(uuid("91493473-5a91-11cf-8700-00aa0060263b"))
SoundFormat : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall Play ( ) = 0;
      virtual HRESULT __stdcall Import (
        /*[in]*/ BSTR FileName ) = 0;
      virtual HRESULT __stdcall Export (
        /*[in]*/ BSTR FileName,
        /*[out,retval]*/ enum PpSoundFormatType * Export ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum PpSoundFormatType * Type ) = 0;
      virtual HRESULT __stdcall get_SourceFullName (
        /*[out,retval]*/ BSTR * SourceFullName ) = 0;
};

struct __declspec(uuid("91493477-5a91-11cf-8700-00aa0060263b"))
PlaceholderFormat : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum PpPlaceholderType * Type ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall put_Name (
        /*[in]*/ BSTR Name ) = 0;
      virtual HRESULT __stdcall get_ContainedType (
        /*[out,retval]*/ enum Office2010::MsoShapeType * ContainedType ) = 0;
};

struct __declspec(uuid("9149347c-5a91-11cf-8700-00aa0060263b"))
Adjustments : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ int * Count ) = 0;
      virtual HRESULT __stdcall get_Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ float * Val ) = 0;
      virtual HRESULT __stdcall put_Item (
        /*[in]*/ int Index,
        /*[in]*/ float Val ) = 0;
};

struct __declspec(uuid("9149347d-5a91-11cf-8700-00aa0060263b"))
PictureFormat : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall IncrementBrightness (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall IncrementContrast (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall get_Brightness (
        /*[out,retval]*/ float * Brightness ) = 0;
      virtual HRESULT __stdcall put_Brightness (
        /*[in]*/ float Brightness ) = 0;
      virtual HRESULT __stdcall get_ColorType (
        /*[out,retval]*/ enum Office2010::MsoPictureColorType * ColorType ) = 0;
      virtual HRESULT __stdcall put_ColorType (
        /*[in]*/ enum Office2010::MsoPictureColorType ColorType ) = 0;
      virtual HRESULT __stdcall get_Contrast (
        /*[out,retval]*/ float * Contrast ) = 0;
      virtual HRESULT __stdcall put_Contrast (
        /*[in]*/ float Contrast ) = 0;
      virtual HRESULT __stdcall get_CropBottom (
        /*[out,retval]*/ float * CropBottom ) = 0;
      virtual HRESULT __stdcall put_CropBottom (
        /*[in]*/ float CropBottom ) = 0;
      virtual HRESULT __stdcall get_CropLeft (
        /*[out,retval]*/ float * CropLeft ) = 0;
      virtual HRESULT __stdcall put_CropLeft (
        /*[in]*/ float CropLeft ) = 0;
      virtual HRESULT __stdcall get_CropRight (
        /*[out,retval]*/ float * CropRight ) = 0;
      virtual HRESULT __stdcall put_CropRight (
        /*[in]*/ float CropRight ) = 0;
      virtual HRESULT __stdcall get_CropTop (
        /*[out,retval]*/ float * CropTop ) = 0;
      virtual HRESULT __stdcall put_CropTop (
        /*[in]*/ float CropTop ) = 0;
      virtual HRESULT __stdcall get_TransparencyColor (
        /*[out,retval]*/ Office2010::MsoRGBType * TransparencyColor ) = 0;
      virtual HRESULT __stdcall put_TransparencyColor (
        /*[in]*/ Office2010::MsoRGBType TransparencyColor ) = 0;
      virtual HRESULT __stdcall get_TransparentBackground (
        /*[out,retval]*/ enum Office2010::MsoTriState * TransparentBackground ) = 0;
      virtual HRESULT __stdcall put_TransparentBackground (
        /*[in]*/ enum Office2010::MsoTriState TransparentBackground ) = 0;
      virtual HRESULT __stdcall get_Crop (
        /*[out,retval]*/ struct Office2010::Crop * * Crop ) = 0;
};

struct __declspec(uuid("9149347e-5a91-11cf-8700-00aa0060263b"))
FillFormat : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Background ( ) = 0;
      virtual HRESULT __stdcall OneColorGradient (
        /*[in]*/ enum Office2010::MsoGradientStyle Style,
        /*[in]*/ int Variant,
        /*[in]*/ float Degree ) = 0;
      virtual HRESULT __stdcall Patterned (
        /*[in]*/ enum Office2010::MsoPatternType Pattern ) = 0;
      virtual HRESULT __stdcall PresetGradient (
        /*[in]*/ enum Office2010::MsoGradientStyle Style,
        /*[in]*/ int Variant,
        /*[in]*/ enum Office2010::MsoPresetGradientType PresetGradientType ) = 0;
      virtual HRESULT __stdcall PresetTextured (
        /*[in]*/ enum Office2010::MsoPresetTexture PresetTexture ) = 0;
      virtual HRESULT __stdcall Solid ( ) = 0;
      virtual HRESULT __stdcall TwoColorGradient (
        /*[in]*/ enum Office2010::MsoGradientStyle Style,
        /*[in]*/ int Variant ) = 0;
      virtual HRESULT __stdcall UserPicture (
        /*[in]*/ BSTR PictureFile ) = 0;
      virtual HRESULT __stdcall UserTextured (
        /*[in]*/ BSTR TextureFile ) = 0;
      virtual HRESULT __stdcall get_BackColor (
        /*[out,retval]*/ struct ColorFormat * * BackColor ) = 0;
      virtual HRESULT __stdcall put_BackColor (
        /*[in]*/ struct ColorFormat * BackColor ) = 0;
      virtual HRESULT __stdcall get_ForeColor (
        /*[out,retval]*/ struct ColorFormat * * ForeColor ) = 0;
      virtual HRESULT __stdcall put_ForeColor (
        /*[in]*/ struct ColorFormat * ForeColor ) = 0;
      virtual HRESULT __stdcall get_GradientColorType (
        /*[out,retval]*/ enum Office2010::MsoGradientColorType * GradientColorType ) = 0;
      virtual HRESULT __stdcall get_GradientDegree (
        /*[out,retval]*/ float * GradientDegree ) = 0;
      virtual HRESULT __stdcall get_GradientStyle (
        /*[out,retval]*/ enum Office2010::MsoGradientStyle * GradientStyle ) = 0;
      virtual HRESULT __stdcall get_GradientVariant (
        /*[out,retval]*/ int * GradientVariant ) = 0;
      virtual HRESULT __stdcall get_Pattern (
        /*[out,retval]*/ enum Office2010::MsoPatternType * Pattern ) = 0;
      virtual HRESULT __stdcall get_PresetGradientType (
        /*[out,retval]*/ enum Office2010::MsoPresetGradientType * PresetGradientType ) = 0;
      virtual HRESULT __stdcall get_PresetTexture (
        /*[out,retval]*/ enum Office2010::MsoPresetTexture * PresetTexture ) = 0;
      virtual HRESULT __stdcall get_TextureName (
        /*[out,retval]*/ BSTR * TextureName ) = 0;
      virtual HRESULT __stdcall get_TextureType (
        /*[out,retval]*/ enum Office2010::MsoTextureType * TextureType ) = 0;
      virtual HRESULT __stdcall get_Transparency (
        /*[out,retval]*/ float * Transparency ) = 0;
      virtual HRESULT __stdcall put_Transparency (
        /*[in]*/ float Transparency ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum Office2010::MsoFillType * Type ) = 0;
      virtual HRESULT __stdcall get_Visible (
        /*[out,retval]*/ enum Office2010::MsoTriState * Visible ) = 0;
      virtual HRESULT __stdcall put_Visible (
        /*[in]*/ enum Office2010::MsoTriState Visible ) = 0;
      virtual HRESULT __stdcall get_GradientStops (
        /*[out,retval]*/ struct Office2010::GradientStops * * GradientStops ) = 0;
      virtual HRESULT __stdcall get_TextureOffsetX (
        /*[out,retval]*/ float * TextureOffsetX ) = 0;
      virtual HRESULT __stdcall put_TextureOffsetX (
        /*[in]*/ float TextureOffsetX ) = 0;
      virtual HRESULT __stdcall get_TextureOffsetY (
        /*[out,retval]*/ float * TextureOffsetY ) = 0;
      virtual HRESULT __stdcall put_TextureOffsetY (
        /*[in]*/ float TextureOffsetY ) = 0;
      virtual HRESULT __stdcall get_TextureAlignment (
        /*[out,retval]*/ enum Office2010::MsoTextureAlignment * TextureAlignment ) = 0;
      virtual HRESULT __stdcall put_TextureAlignment (
        /*[in]*/ enum Office2010::MsoTextureAlignment TextureAlignment ) = 0;
      virtual HRESULT __stdcall get_TextureHorizontalScale (
        /*[out,retval]*/ float * HorizontalScale ) = 0;
      virtual HRESULT __stdcall put_TextureHorizontalScale (
        /*[in]*/ float HorizontalScale ) = 0;
      virtual HRESULT __stdcall get_TextureVerticalScale (
        /*[out,retval]*/ float * VerticalScale ) = 0;
      virtual HRESULT __stdcall put_TextureVerticalScale (
        /*[in]*/ float VerticalScale ) = 0;
      virtual HRESULT __stdcall get_TextureTile (
        /*[out,retval]*/ enum Office2010::MsoTriState * TextureTile ) = 0;
      virtual HRESULT __stdcall put_TextureTile (
        /*[in]*/ enum Office2010::MsoTriState TextureTile ) = 0;
      virtual HRESULT __stdcall get_RotateWithObject (
        /*[out,retval]*/ enum Office2010::MsoTriState * RotateWithObject ) = 0;
      virtual HRESULT __stdcall put_RotateWithObject (
        /*[in]*/ enum Office2010::MsoTriState RotateWithObject ) = 0;
      virtual HRESULT __stdcall get_PictureEffects (
        /*[out,retval]*/ struct Office2010::PictureEffects * * PictureEffects ) = 0;
      virtual HRESULT __stdcall get_GradientAngle (
        /*[out,retval]*/ float * GradientAngle ) = 0;
      virtual HRESULT __stdcall put_GradientAngle (
        /*[in]*/ float GradientAngle ) = 0;
};

struct __declspec(uuid("9149347f-5a91-11cf-8700-00aa0060263b"))
LineFormat : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_BackColor (
        /*[out,retval]*/ struct ColorFormat * * BackColor ) = 0;
      virtual HRESULT __stdcall put_BackColor (
        /*[in]*/ struct ColorFormat * BackColor ) = 0;
      virtual HRESULT __stdcall get_BeginArrowheadLength (
        /*[out,retval]*/ enum Office2010::MsoArrowheadLength * BeginArrowheadLength ) = 0;
      virtual HRESULT __stdcall put_BeginArrowheadLength (
        /*[in]*/ enum Office2010::MsoArrowheadLength BeginArrowheadLength ) = 0;
      virtual HRESULT __stdcall get_BeginArrowheadStyle (
        /*[out,retval]*/ enum Office2010::MsoArrowheadStyle * BeginArrowheadStyle ) = 0;
      virtual HRESULT __stdcall put_BeginArrowheadStyle (
        /*[in]*/ enum Office2010::MsoArrowheadStyle BeginArrowheadStyle ) = 0;
      virtual HRESULT __stdcall get_BeginArrowheadWidth (
        /*[out,retval]*/ enum Office2010::MsoArrowheadWidth * BeginArrowheadWidth ) = 0;
      virtual HRESULT __stdcall put_BeginArrowheadWidth (
        /*[in]*/ enum Office2010::MsoArrowheadWidth BeginArrowheadWidth ) = 0;
      virtual HRESULT __stdcall get_DashStyle (
        /*[out,retval]*/ enum Office2010::MsoLineDashStyle * DashStyle ) = 0;
      virtual HRESULT __stdcall put_DashStyle (
        /*[in]*/ enum Office2010::MsoLineDashStyle DashStyle ) = 0;
      virtual HRESULT __stdcall get_EndArrowheadLength (
        /*[out,retval]*/ enum Office2010::MsoArrowheadLength * EndArrowheadLength ) = 0;
      virtual HRESULT __stdcall put_EndArrowheadLength (
        /*[in]*/ enum Office2010::MsoArrowheadLength EndArrowheadLength ) = 0;
      virtual HRESULT __stdcall get_EndArrowheadStyle (
        /*[out,retval]*/ enum Office2010::MsoArrowheadStyle * EndArrowheadStyle ) = 0;
      virtual HRESULT __stdcall put_EndArrowheadStyle (
        /*[in]*/ enum Office2010::MsoArrowheadStyle EndArrowheadStyle ) = 0;
      virtual HRESULT __stdcall get_EndArrowheadWidth (
        /*[out,retval]*/ enum Office2010::MsoArrowheadWidth * EndArrowheadWidth ) = 0;
      virtual HRESULT __stdcall put_EndArrowheadWidth (
        /*[in]*/ enum Office2010::MsoArrowheadWidth EndArrowheadWidth ) = 0;
      virtual HRESULT __stdcall get_ForeColor (
        /*[out,retval]*/ struct ColorFormat * * ForeColor ) = 0;
      virtual HRESULT __stdcall put_ForeColor (
        /*[in]*/ struct ColorFormat * ForeColor ) = 0;
      virtual HRESULT __stdcall get_Pattern (
        /*[out,retval]*/ enum Office2010::MsoPatternType * Pattern ) = 0;
      virtual HRESULT __stdcall put_Pattern (
        /*[in]*/ enum Office2010::MsoPatternType Pattern ) = 0;
      virtual HRESULT __stdcall get_Style (
        /*[out,retval]*/ enum Office2010::MsoLineStyle * Style ) = 0;
      virtual HRESULT __stdcall put_Style (
        /*[in]*/ enum Office2010::MsoLineStyle Style ) = 0;
      virtual HRESULT __stdcall get_Transparency (
        /*[out,retval]*/ float * Transparency ) = 0;
      virtual HRESULT __stdcall put_Transparency (
        /*[in]*/ float Transparency ) = 0;
      virtual HRESULT __stdcall get_Visible (
        /*[out,retval]*/ enum Office2010::MsoTriState * Visible ) = 0;
      virtual HRESULT __stdcall put_Visible (
        /*[in]*/ enum Office2010::MsoTriState Visible ) = 0;
      virtual HRESULT __stdcall get_Weight (
        /*[out,retval]*/ float * Weight ) = 0;
      virtual HRESULT __stdcall put_Weight (
        /*[in]*/ float Weight ) = 0;
      virtual HRESULT __stdcall get_InsetPen (
        /*[out,retval]*/ enum Office2010::MsoTriState * InsetPen ) = 0;
      virtual HRESULT __stdcall put_InsetPen (
        /*[in]*/ enum Office2010::MsoTriState InsetPen ) = 0;
};

struct __declspec(uuid("91493480-5a91-11cf-8700-00aa0060263b"))
ShadowFormat : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall IncrementOffsetX (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall IncrementOffsetY (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall get_ForeColor (
        /*[out,retval]*/ struct ColorFormat * * ForeColor ) = 0;
      virtual HRESULT __stdcall put_ForeColor (
        /*[in]*/ struct ColorFormat * ForeColor ) = 0;
      virtual HRESULT __stdcall get_Obscured (
        /*[out,retval]*/ enum Office2010::MsoTriState * Obscured ) = 0;
      virtual HRESULT __stdcall put_Obscured (
        /*[in]*/ enum Office2010::MsoTriState Obscured ) = 0;
      virtual HRESULT __stdcall get_OffsetX (
        /*[out,retval]*/ float * OffsetX ) = 0;
      virtual HRESULT __stdcall put_OffsetX (
        /*[in]*/ float OffsetX ) = 0;
      virtual HRESULT __stdcall get_OffsetY (
        /*[out,retval]*/ float * OffsetY ) = 0;
      virtual HRESULT __stdcall put_OffsetY (
        /*[in]*/ float OffsetY ) = 0;
      virtual HRESULT __stdcall get_Transparency (
        /*[out,retval]*/ float * Transparency ) = 0;
      virtual HRESULT __stdcall put_Transparency (
        /*[in]*/ float Transparency ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum Office2010::MsoShadowType * Type ) = 0;
      virtual HRESULT __stdcall put_Type (
        /*[in]*/ enum Office2010::MsoShadowType Type ) = 0;
      virtual HRESULT __stdcall get_Visible (
        /*[out,retval]*/ enum Office2010::MsoTriState * Visible ) = 0;
      virtual HRESULT __stdcall put_Visible (
        /*[in]*/ enum Office2010::MsoTriState Visible ) = 0;
      virtual HRESULT __stdcall get_Style (
        /*[out,retval]*/ enum Office2010::MsoShadowStyle * ShadowStyle ) = 0;
      virtual HRESULT __stdcall put_Style (
        /*[in]*/ enum Office2010::MsoShadowStyle ShadowStyle ) = 0;
      virtual HRESULT __stdcall get_Blur (
        /*[out,retval]*/ float * Blur ) = 0;
      virtual HRESULT __stdcall put_Blur (
        /*[in]*/ float Blur ) = 0;
      virtual HRESULT __stdcall get_Size (
        /*[out,retval]*/ float * Size ) = 0;
      virtual HRESULT __stdcall put_Size (
        /*[in]*/ float Size ) = 0;
      virtual HRESULT __stdcall get_RotateWithShape (
        /*[out,retval]*/ enum Office2010::MsoTriState * RotateWithShape ) = 0;
      virtual HRESULT __stdcall put_RotateWithShape (
        /*[in]*/ enum Office2010::MsoTriState RotateWithShape ) = 0;
};

struct __declspec(uuid("91493482-5a91-11cf-8700-00aa0060263b"))
TextEffectFormat : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall ToggleVerticalText ( ) = 0;
      virtual HRESULT __stdcall get_Alignment (
        /*[out,retval]*/ enum Office2010::MsoTextEffectAlignment * Alignment ) = 0;
      virtual HRESULT __stdcall put_Alignment (
        /*[in]*/ enum Office2010::MsoTextEffectAlignment Alignment ) = 0;
      virtual HRESULT __stdcall get_FontBold (
        /*[out,retval]*/ enum Office2010::MsoTriState * FontBold ) = 0;
      virtual HRESULT __stdcall put_FontBold (
        /*[in]*/ enum Office2010::MsoTriState FontBold ) = 0;
      virtual HRESULT __stdcall get_FontItalic (
        /*[out,retval]*/ enum Office2010::MsoTriState * FontItalic ) = 0;
      virtual HRESULT __stdcall put_FontItalic (
        /*[in]*/ enum Office2010::MsoTriState FontItalic ) = 0;
      virtual HRESULT __stdcall get_FontName (
        /*[out,retval]*/ BSTR * FontName ) = 0;
      virtual HRESULT __stdcall put_FontName (
        /*[in]*/ BSTR FontName ) = 0;
      virtual HRESULT __stdcall get_FontSize (
        /*[out,retval]*/ float * FontSize ) = 0;
      virtual HRESULT __stdcall put_FontSize (
        /*[in]*/ float FontSize ) = 0;
      virtual HRESULT __stdcall get_KernedPairs (
        /*[out,retval]*/ enum Office2010::MsoTriState * KernedPairs ) = 0;
      virtual HRESULT __stdcall put_KernedPairs (
        /*[in]*/ enum Office2010::MsoTriState KernedPairs ) = 0;
      virtual HRESULT __stdcall get_NormalizedHeight (
        /*[out,retval]*/ enum Office2010::MsoTriState * NormalizedHeight ) = 0;
      virtual HRESULT __stdcall put_NormalizedHeight (
        /*[in]*/ enum Office2010::MsoTriState NormalizedHeight ) = 0;
      virtual HRESULT __stdcall get_PresetShape (
        /*[out,retval]*/ enum Office2010::MsoPresetTextEffectShape * PresetShape ) = 0;
      virtual HRESULT __stdcall put_PresetShape (
        /*[in]*/ enum Office2010::MsoPresetTextEffectShape PresetShape ) = 0;
      virtual HRESULT __stdcall get_PresetTextEffect (
        /*[out,retval]*/ enum Office2010::MsoPresetTextEffect * Preset ) = 0;
      virtual HRESULT __stdcall put_PresetTextEffect (
        /*[in]*/ enum Office2010::MsoPresetTextEffect Preset ) = 0;
      virtual HRESULT __stdcall get_RotatedChars (
        /*[out,retval]*/ enum Office2010::MsoTriState * RotatedChars ) = 0;
      virtual HRESULT __stdcall put_RotatedChars (
        /*[in]*/ enum Office2010::MsoTriState RotatedChars ) = 0;
      virtual HRESULT __stdcall get_Text (
        /*[out,retval]*/ BSTR * Text ) = 0;
      virtual HRESULT __stdcall put_Text (
        /*[in]*/ BSTR Text ) = 0;
      virtual HRESULT __stdcall get_Tracking (
        /*[out,retval]*/ float * Tracking ) = 0;
      virtual HRESULT __stdcall put_Tracking (
        /*[in]*/ float Tracking ) = 0;
};

struct __declspec(uuid("91493483-5a91-11cf-8700-00aa0060263b"))
ThreeDFormat : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall IncrementRotationX (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall IncrementRotationY (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall ResetRotation ( ) = 0;
      virtual HRESULT __stdcall SetThreeDFormat (
        /*[in]*/ enum Office2010::MsoPresetThreeDFormat PresetThreeDFormat ) = 0;
      virtual HRESULT __stdcall SetExtrusionDirection (
        /*[in]*/ enum Office2010::MsoPresetExtrusionDirection PresetExtrusionDirection ) = 0;
      virtual HRESULT __stdcall get_Depth (
        /*[out,retval]*/ float * Depth ) = 0;
      virtual HRESULT __stdcall put_Depth (
        /*[in]*/ float Depth ) = 0;
      virtual HRESULT __stdcall get_ExtrusionColor (
        /*[out,retval]*/ struct ColorFormat * * ExtrusionColor ) = 0;
      virtual HRESULT __stdcall get_ExtrusionColorType (
        /*[out,retval]*/ enum Office2010::MsoExtrusionColorType * ExtrusionColorType ) = 0;
      virtual HRESULT __stdcall put_ExtrusionColorType (
        /*[in]*/ enum Office2010::MsoExtrusionColorType ExtrusionColorType ) = 0;
      virtual HRESULT __stdcall get_Perspective (
        /*[out,retval]*/ enum Office2010::MsoTriState * Perspective ) = 0;
      virtual HRESULT __stdcall put_Perspective (
        /*[in]*/ enum Office2010::MsoTriState Perspective ) = 0;
      virtual HRESULT __stdcall get_PresetExtrusionDirection (
        /*[out,retval]*/ enum Office2010::MsoPresetExtrusionDirection * PresetExtrusionDirection ) = 0;
      virtual HRESULT __stdcall get_PresetLightingDirection (
        /*[out,retval]*/ enum Office2010::MsoPresetLightingDirection * PresetLightingDirection ) = 0;
      virtual HRESULT __stdcall put_PresetLightingDirection (
        /*[in]*/ enum Office2010::MsoPresetLightingDirection PresetLightingDirection ) = 0;
      virtual HRESULT __stdcall get_PresetLightingSoftness (
        /*[out,retval]*/ enum Office2010::MsoPresetLightingSoftness * PresetLightingSoftness ) = 0;
      virtual HRESULT __stdcall put_PresetLightingSoftness (
        /*[in]*/ enum Office2010::MsoPresetLightingSoftness PresetLightingSoftness ) = 0;
      virtual HRESULT __stdcall get_PresetMaterial (
        /*[out,retval]*/ enum Office2010::MsoPresetMaterial * PresetMaterial ) = 0;
      virtual HRESULT __stdcall put_PresetMaterial (
        /*[in]*/ enum Office2010::MsoPresetMaterial PresetMaterial ) = 0;
      virtual HRESULT __stdcall get_PresetThreeDFormat (
        /*[out,retval]*/ enum Office2010::MsoPresetThreeDFormat * PresetThreeDFormat ) = 0;
      virtual HRESULT __stdcall get_RotationX (
        /*[out,retval]*/ float * RotationX ) = 0;
      virtual HRESULT __stdcall put_RotationX (
        /*[in]*/ float RotationX ) = 0;
      virtual HRESULT __stdcall get_RotationY (
        /*[out,retval]*/ float * RotationY ) = 0;
      virtual HRESULT __stdcall put_RotationY (
        /*[in]*/ float RotationY ) = 0;
      virtual HRESULT __stdcall get_Visible (
        /*[out,retval]*/ enum Office2010::MsoTriState * Visible ) = 0;
      virtual HRESULT __stdcall put_Visible (
        /*[in]*/ enum Office2010::MsoTriState Visible ) = 0;
      virtual HRESULT __stdcall SetPresetCamera (
        /*[in]*/ enum Office2010::MsoPresetCamera PresetCamera ) = 0;
      virtual HRESULT __stdcall IncrementRotationZ (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall IncrementRotationHorizontal (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall IncrementRotationVertical (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall get_PresetLighting (
        /*[out,retval]*/ enum Office2010::MsoLightRigType * PresetLightRigType ) = 0;
      virtual HRESULT __stdcall put_PresetLighting (
        /*[in]*/ enum Office2010::MsoLightRigType PresetLightRigType ) = 0;
      virtual HRESULT __stdcall get_Z (
        /*[out,retval]*/ float * Z ) = 0;
      virtual HRESULT __stdcall put_Z (
        /*[in]*/ float Z ) = 0;
      virtual HRESULT __stdcall get_BevelTopType (
        /*[out,retval]*/ enum Office2010::MsoBevelType * BevelTopType ) = 0;
      virtual HRESULT __stdcall put_BevelTopType (
        /*[in]*/ enum Office2010::MsoBevelType BevelTopType ) = 0;
      virtual HRESULT __stdcall get_BevelTopInset (
        /*[out,retval]*/ float * BevelTopInset ) = 0;
      virtual HRESULT __stdcall put_BevelTopInset (
        /*[in]*/ float BevelTopInset ) = 0;
      virtual HRESULT __stdcall get_BevelTopDepth (
        /*[out,retval]*/ float * BevelTopDepth ) = 0;
      virtual HRESULT __stdcall put_BevelTopDepth (
        /*[in]*/ float BevelTopDepth ) = 0;
      virtual HRESULT __stdcall get_BevelBottomType (
        /*[out,retval]*/ enum Office2010::MsoBevelType * BevelBottomType ) = 0;
      virtual HRESULT __stdcall put_BevelBottomType (
        /*[in]*/ enum Office2010::MsoBevelType BevelBottomType ) = 0;
      virtual HRESULT __stdcall get_BevelBottomInset (
        /*[out,retval]*/ float * BevelBottomInset ) = 0;
      virtual HRESULT __stdcall put_BevelBottomInset (
        /*[in]*/ float BevelBottomInset ) = 0;
      virtual HRESULT __stdcall get_BevelBottomDepth (
        /*[out,retval]*/ float * BevelBottomDepth ) = 0;
      virtual HRESULT __stdcall put_BevelBottomDepth (
        /*[in]*/ float BevelBottomDepth ) = 0;
      virtual HRESULT __stdcall get_PresetCamera (
        /*[out,retval]*/ enum Office2010::MsoPresetCamera * PresetCamera ) = 0;
      virtual HRESULT __stdcall get_RotationZ (
        /*[out,retval]*/ float * RotationZ ) = 0;
      virtual HRESULT __stdcall put_RotationZ (
        /*[in]*/ float RotationZ ) = 0;
      virtual HRESULT __stdcall get_ContourWidth (
        /*[out,retval]*/ float * Width ) = 0;
      virtual HRESULT __stdcall put_ContourWidth (
        /*[in]*/ float Width ) = 0;
      virtual HRESULT __stdcall get_ContourColor (
        /*[out,retval]*/ struct ColorFormat * * ContourColor ) = 0;
      virtual HRESULT __stdcall get_FieldOfView (
        /*[out,retval]*/ float * FOV ) = 0;
      virtual HRESULT __stdcall put_FieldOfView (
        /*[in]*/ float FOV ) = 0;
      virtual HRESULT __stdcall get_ProjectText (
        /*[out,retval]*/ enum Office2010::MsoTriState * ProjectText ) = 0;
      virtual HRESULT __stdcall put_ProjectText (
        /*[in]*/ enum Office2010::MsoTriState ProjectText ) = 0;
      virtual HRESULT __stdcall get_LightAngle (
        /*[out,retval]*/ float * LightAngle ) = 0;
      virtual HRESULT __stdcall put_LightAngle (
        /*[in]*/ float LightAngle ) = 0;
};

struct __declspec(uuid("91493485-5a91-11cf-8700-00aa0060263b"))
CalloutFormat : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall AutomaticLength ( ) = 0;
      virtual HRESULT __stdcall CustomDrop (
        /*[in]*/ float Drop ) = 0;
      virtual HRESULT __stdcall CustomLength (
        /*[in]*/ float Length ) = 0;
      virtual HRESULT __stdcall PresetDrop (
        /*[in]*/ enum Office2010::MsoCalloutDropType DropType ) = 0;
      virtual HRESULT __stdcall get_Accent (
        /*[out,retval]*/ enum Office2010::MsoTriState * Accent ) = 0;
      virtual HRESULT __stdcall put_Accent (
        /*[in]*/ enum Office2010::MsoTriState Accent ) = 0;
      virtual HRESULT __stdcall get_Angle (
        /*[out,retval]*/ enum Office2010::MsoCalloutAngleType * Angle ) = 0;
      virtual HRESULT __stdcall put_Angle (
        /*[in]*/ enum Office2010::MsoCalloutAngleType Angle ) = 0;
      virtual HRESULT __stdcall get_AutoAttach (
        /*[out,retval]*/ enum Office2010::MsoTriState * AutoAttach ) = 0;
      virtual HRESULT __stdcall put_AutoAttach (
        /*[in]*/ enum Office2010::MsoTriState AutoAttach ) = 0;
      virtual HRESULT __stdcall get_AutoLength (
        /*[out,retval]*/ enum Office2010::MsoTriState * AutoLength ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ enum Office2010::MsoTriState * Border ) = 0;
      virtual HRESULT __stdcall put_Border (
        /*[in]*/ enum Office2010::MsoTriState Border ) = 0;
      virtual HRESULT __stdcall get_Drop (
        /*[out,retval]*/ float * Drop ) = 0;
      virtual HRESULT __stdcall get_DropType (
        /*[out,retval]*/ enum Office2010::MsoCalloutDropType * DropType ) = 0;
      virtual HRESULT __stdcall get_Gap (
        /*[out,retval]*/ float * Gap ) = 0;
      virtual HRESULT __stdcall put_Gap (
        /*[in]*/ float Gap ) = 0;
      virtual HRESULT __stdcall get_Length (
        /*[out,retval]*/ float * Length ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum Office2010::MsoCalloutType * Type ) = 0;
      virtual HRESULT __stdcall put_Type (
        /*[in]*/ enum Office2010::MsoCalloutType Type ) = 0;
};

struct __declspec(uuid("91493487-5a91-11cf-8700-00aa0060263b"))
ShapeNode : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_EditingType (
        /*[out,retval]*/ enum Office2010::MsoEditingType * EditingType ) = 0;
      virtual HRESULT __stdcall get_Points (
        /*[out,retval]*/ VARIANT * Points ) = 0;
      virtual HRESULT __stdcall get_SegmentType (
        /*[out,retval]*/ enum Office2010::MsoSegmentType * SegmentType ) = 0;
};

struct __declspec(uuid("91493486-5a91-11cf-8700-00aa0060263b"))
ShapeNodes : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ int * Count ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct ShapeNode * * Item ) = 0;
      virtual HRESULT __stdcall get__NewEnum (
        /*[out,retval]*/ IUnknown * * _NewEnum ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[in]*/ int Index ) = 0;
      virtual HRESULT __stdcall Insert (
        /*[in]*/ int Index,
        /*[in]*/ enum Office2010::MsoSegmentType SegmentType,
        /*[in]*/ enum Office2010::MsoEditingType EditingType,
        /*[in]*/ float X1,
        /*[in]*/ float Y1,
        /*[in]*/ float X2,
        /*[in]*/ float Y2,
        /*[in]*/ float X3,
        /*[in]*/ float Y3 ) = 0;
      virtual HRESULT __stdcall SetEditingType (
        /*[in]*/ int Index,
        /*[in]*/ enum Office2010::MsoEditingType EditingType ) = 0;
      virtual HRESULT __stdcall SetPosition (
        /*[in]*/ int Index,
        /*[in]*/ float X1,
        /*[in]*/ float Y1 ) = 0;
      virtual HRESULT __stdcall SetSegmentType (
        /*[in]*/ int Index,
        /*[in]*/ enum Office2010::MsoSegmentType SegmentType ) = 0;
};

struct __declspec(uuid("91493489-5a91-11cf-8700-00aa0060263b"))
LinkFormat : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_SourceFullName (
        /*[out,retval]*/ BSTR * SourceFullName ) = 0;
      virtual HRESULT __stdcall put_SourceFullName (
        /*[in]*/ BSTR SourceFullName ) = 0;
      virtual HRESULT __stdcall get_AutoUpdate (
        /*[out,retval]*/ enum PpUpdateOption * AutoUpdate ) = 0;
      virtual HRESULT __stdcall put_AutoUpdate (
        /*[in]*/ enum PpUpdateOption AutoUpdate ) = 0;
      virtual HRESULT __stdcall Update ( ) = 0;
      virtual HRESULT __stdcall BreakLink ( ) = 0;
};

struct __declspec(uuid("9149348a-5a91-11cf-8700-00aa0060263b"))
ObjectVerbs : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ BSTR * Item ) = 0;
};

struct __declspec(uuid("91493488-5a91-11cf-8700-00aa0060263b"))
OLEFormat : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_ObjectVerbs (
        /*[out,retval]*/ struct ObjectVerbs * * ObjectVerbs ) = 0;
      virtual HRESULT __stdcall get_Object (
        /*[out,retval]*/ IDispatch * * Object ) = 0;
      virtual HRESULT __stdcall get_ProgID (
        /*[out,retval]*/ BSTR * ProgID ) = 0;
      virtual HRESULT __stdcall get_FollowColors (
        /*[out,retval]*/ enum PpFollowColors * FollowColors ) = 0;
      virtual HRESULT __stdcall put_FollowColors (
        /*[in]*/ enum PpFollowColors FollowColors ) = 0;
      virtual HRESULT __stdcall DoVerb (
        /*[in]*/ int Index ) = 0;
      virtual HRESULT __stdcall Activate ( ) = 0;
};

struct __declspec(uuid("9149348d-5a91-11cf-8700-00aa0060263b"))
ActionSetting : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Action (
        /*[out,retval]*/ enum PpActionType * Action ) = 0;
      virtual HRESULT __stdcall put_Action (
        /*[in]*/ enum PpActionType Action ) = 0;
      virtual HRESULT __stdcall get_ActionVerb (
        /*[out,retval]*/ BSTR * ActionVerb ) = 0;
      virtual HRESULT __stdcall put_ActionVerb (
        /*[in]*/ BSTR ActionVerb ) = 0;
      virtual HRESULT __stdcall get_AnimateAction (
        /*[out,retval]*/ enum Office2010::MsoTriState * AnimateAction ) = 0;
      virtual HRESULT __stdcall put_AnimateAction (
        /*[in]*/ enum Office2010::MsoTriState AnimateAction ) = 0;
      virtual HRESULT __stdcall get_Run (
        /*[out,retval]*/ BSTR * Run ) = 0;
      virtual HRESULT __stdcall put_Run (
        /*[in]*/ BSTR Run ) = 0;
      virtual HRESULT __stdcall get_SlideShowName (
        /*[out,retval]*/ BSTR * SlideShowName ) = 0;
      virtual HRESULT __stdcall put_SlideShowName (
        /*[in]*/ BSTR SlideShowName ) = 0;
      virtual HRESULT __stdcall get_Hyperlink (
        /*[out,retval]*/ struct Hyperlink * * Hyperlink ) = 0;
      virtual HRESULT __stdcall get_SoundEffect (
        /*[out,retval]*/ struct SoundEffect * * SoundEffect ) = 0;
      virtual HRESULT __stdcall get_ShowAndReturn (
        /*[out,retval]*/ enum Office2010::MsoTriState * ShowAndReturn ) = 0;
      virtual HRESULT __stdcall put_ShowAndReturn (
        /*[in]*/ enum Office2010::MsoTriState ShowAndReturn ) = 0;
};

struct __declspec(uuid("9149348c-5a91-11cf-8700-00aa0060263b"))
ActionSettings : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ enum PpMouseActivation Index,
        /*[out,retval]*/ struct ActionSetting * * Item ) = 0;
};

struct __declspec(uuid("9149348e-5a91-11cf-8700-00aa0060263b"))
PlaySettings : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_ActionVerb (
        /*[out,retval]*/ BSTR * ActionVerb ) = 0;
      virtual HRESULT __stdcall put_ActionVerb (
        /*[in]*/ BSTR ActionVerb ) = 0;
      virtual HRESULT __stdcall get_HideWhileNotPlaying (
        /*[out,retval]*/ enum Office2010::MsoTriState * HideWhileNotPlaying ) = 0;
      virtual HRESULT __stdcall put_HideWhileNotPlaying (
        /*[in]*/ enum Office2010::MsoTriState HideWhileNotPlaying ) = 0;
      virtual HRESULT __stdcall get_LoopUntilStopped (
        /*[out,retval]*/ enum Office2010::MsoTriState * LoopUntilStopped ) = 0;
      virtual HRESULT __stdcall put_LoopUntilStopped (
        /*[in]*/ enum Office2010::MsoTriState LoopUntilStopped ) = 0;
      virtual HRESULT __stdcall get_PlayOnEntry (
        /*[out,retval]*/ enum Office2010::MsoTriState * PlayOnEntry ) = 0;
      virtual HRESULT __stdcall put_PlayOnEntry (
        /*[in]*/ enum Office2010::MsoTriState PlayOnEntry ) = 0;
      virtual HRESULT __stdcall get_RewindMovie (
        /*[out,retval]*/ enum Office2010::MsoTriState * RewindMovie ) = 0;
      virtual HRESULT __stdcall put_RewindMovie (
        /*[in]*/ enum Office2010::MsoTriState RewindMovie ) = 0;
      virtual HRESULT __stdcall get_PauseAnimation (
        /*[out,retval]*/ enum Office2010::MsoTriState * PauseAnimation ) = 0;
      virtual HRESULT __stdcall put_PauseAnimation (
        /*[in]*/ enum Office2010::MsoTriState PauseAnimation ) = 0;
      virtual HRESULT __stdcall get_StopAfterSlides (
        /*[out,retval]*/ int * StopAfterSlides ) = 0;
      virtual HRESULT __stdcall put_StopAfterSlides (
        /*[in]*/ int StopAfterSlides ) = 0;
};

struct __declspec(uuid("9149348b-5a91-11cf-8700-00aa0060263b"))
AnimationSettings : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_DimColor (
        /*[out,retval]*/ struct ColorFormat * * DimColor ) = 0;
      virtual HRESULT __stdcall get_SoundEffect (
        /*[out,retval]*/ struct SoundEffect * * SoundEffect ) = 0;
      virtual HRESULT __stdcall get_EntryEffect (
        /*[out,retval]*/ enum PpEntryEffect * EntryEffect ) = 0;
      virtual HRESULT __stdcall put_EntryEffect (
        /*[in]*/ enum PpEntryEffect EntryEffect ) = 0;
      virtual HRESULT __stdcall get_AfterEffect (
        /*[out,retval]*/ enum PpAfterEffect * AfterEffect ) = 0;
      virtual HRESULT __stdcall put_AfterEffect (
        /*[in]*/ enum PpAfterEffect AfterEffect ) = 0;
      virtual HRESULT __stdcall get_AnimationOrder (
        /*[out,retval]*/ int * AnimationOrder ) = 0;
      virtual HRESULT __stdcall put_AnimationOrder (
        /*[in]*/ int AnimationOrder ) = 0;
      virtual HRESULT __stdcall get_AdvanceMode (
        /*[out,retval]*/ enum PpAdvanceMode * AdvanceMode ) = 0;
      virtual HRESULT __stdcall put_AdvanceMode (
        /*[in]*/ enum PpAdvanceMode AdvanceMode ) = 0;
      virtual HRESULT __stdcall get_AdvanceTime (
        /*[out,retval]*/ float * AdvanceTime ) = 0;
      virtual HRESULT __stdcall put_AdvanceTime (
        /*[in]*/ float AdvanceTime ) = 0;
      virtual HRESULT __stdcall get_PlaySettings (
        /*[out,retval]*/ struct PlaySettings * * PlaySettings ) = 0;
      virtual HRESULT __stdcall get_TextLevelEffect (
        /*[out,retval]*/ enum PpTextLevelEffect * TextLevelEffect ) = 0;
      virtual HRESULT __stdcall put_TextLevelEffect (
        /*[in]*/ enum PpTextLevelEffect TextLevelEffect ) = 0;
      virtual HRESULT __stdcall get_TextUnitEffect (
        /*[out,retval]*/ enum PpTextUnitEffect * TextUnitEffect ) = 0;
      virtual HRESULT __stdcall put_TextUnitEffect (
        /*[in]*/ enum PpTextUnitEffect TextUnitEffect ) = 0;
      virtual HRESULT __stdcall get_Animate (
        /*[out,retval]*/ enum Office2010::MsoTriState * Animate ) = 0;
      virtual HRESULT __stdcall put_Animate (
        /*[in]*/ enum Office2010::MsoTriState Animate ) = 0;
      virtual HRESULT __stdcall get_AnimateBackground (
        /*[out,retval]*/ enum Office2010::MsoTriState * AnimateBackground ) = 0;
      virtual HRESULT __stdcall put_AnimateBackground (
        /*[in]*/ enum Office2010::MsoTriState AnimateBackground ) = 0;
      virtual HRESULT __stdcall get_AnimateTextInReverse (
        /*[out,retval]*/ enum Office2010::MsoTriState * AnimateTextInReverse ) = 0;
      virtual HRESULT __stdcall put_AnimateTextInReverse (
        /*[in]*/ enum Office2010::MsoTriState AnimateTextInReverse ) = 0;
      virtual HRESULT __stdcall get_ChartUnitEffect (
        /*[out,retval]*/ enum PpChartUnitEffect * ChartUnitEffect ) = 0;
      virtual HRESULT __stdcall put_ChartUnitEffect (
        /*[in]*/ enum PpChartUnitEffect ChartUnitEffect ) = 0;
};

struct __declspec(uuid("91493492-5a91-11cf-8700-00aa0060263b"))
RulerLevel : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_FirstMargin (
        /*[out,retval]*/ float * FirstMargin ) = 0;
      virtual HRESULT __stdcall put_FirstMargin (
        /*[in]*/ float FirstMargin ) = 0;
      virtual HRESULT __stdcall get_LeftMargin (
        /*[out,retval]*/ float * LeftMargin ) = 0;
      virtual HRESULT __stdcall put_LeftMargin (
        /*[in]*/ float LeftMargin ) = 0;
};

struct __declspec(uuid("91493491-5a91-11cf-8700-00aa0060263b"))
RulerLevels : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct RulerLevel * * Item ) = 0;
};

struct __declspec(uuid("91493494-5a91-11cf-8700-00aa0060263b"))
TabStop : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum PpTabStopType * Type ) = 0;
      virtual HRESULT __stdcall put_Type (
        /*[in]*/ enum PpTabStopType Type ) = 0;
      virtual HRESULT __stdcall get_Position (
        /*[out,retval]*/ float * Position ) = 0;
      virtual HRESULT __stdcall put_Position (
        /*[in]*/ float Position ) = 0;
      virtual HRESULT __stdcall Clear ( ) = 0;
};

struct __declspec(uuid("91493493-5a91-11cf-8700-00aa0060263b"))
TabStops : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct TabStop * * Item ) = 0;
      virtual HRESULT __stdcall get_DefaultSpacing (
        /*[out,retval]*/ float * DefaultSpacing ) = 0;
      virtual HRESULT __stdcall put_DefaultSpacing (
        /*[in]*/ float DefaultSpacing ) = 0;
      virtual HRESULT __stdcall Add (
        /*[in]*/ enum PpTabStopType Type,
        /*[in]*/ float Position,
        /*[out,retval]*/ struct TabStop * * Add ) = 0;
};

struct __declspec(uuid("91493490-5a91-11cf-8700-00aa0060263b"))
Ruler : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_TabStops (
        /*[out,retval]*/ struct TabStops * * TabStops ) = 0;
      virtual HRESULT __stdcall get_Levels (
        /*[out,retval]*/ struct RulerLevels * * Levels ) = 0;
};

struct __declspec(uuid("91493495-5a91-11cf-8700-00aa0060263b"))
Font : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Color (
        /*[out,retval]*/ struct ColorFormat * * Color ) = 0;
      virtual HRESULT __stdcall get_Bold (
        /*[out,retval]*/ enum Office2010::MsoTriState * Bold ) = 0;
      virtual HRESULT __stdcall put_Bold (
        /*[in]*/ enum Office2010::MsoTriState Bold ) = 0;
      virtual HRESULT __stdcall get_Italic (
        /*[out,retval]*/ enum Office2010::MsoTriState * Italic ) = 0;
      virtual HRESULT __stdcall put_Italic (
        /*[in]*/ enum Office2010::MsoTriState Italic ) = 0;
      virtual HRESULT __stdcall get_Shadow (
        /*[out,retval]*/ enum Office2010::MsoTriState * Shadow ) = 0;
      virtual HRESULT __stdcall put_Shadow (
        /*[in]*/ enum Office2010::MsoTriState Shadow ) = 0;
      virtual HRESULT __stdcall get_Emboss (
        /*[out,retval]*/ enum Office2010::MsoTriState * Emboss ) = 0;
      virtual HRESULT __stdcall put_Emboss (
        /*[in]*/ enum Office2010::MsoTriState Emboss ) = 0;
      virtual HRESULT __stdcall get_Underline (
        /*[out,retval]*/ enum Office2010::MsoTriState * Underline ) = 0;
      virtual HRESULT __stdcall put_Underline (
        /*[in]*/ enum Office2010::MsoTriState Underline ) = 0;
      virtual HRESULT __stdcall get_Subscript (
        /*[out,retval]*/ enum Office2010::MsoTriState * Subscript ) = 0;
      virtual HRESULT __stdcall put_Subscript (
        /*[in]*/ enum Office2010::MsoTriState Subscript ) = 0;
      virtual HRESULT __stdcall get_Superscript (
        /*[out,retval]*/ enum Office2010::MsoTriState * Superscript ) = 0;
      virtual HRESULT __stdcall put_Superscript (
        /*[in]*/ enum Office2010::MsoTriState Superscript ) = 0;
      virtual HRESULT __stdcall get_BaselineOffset (
        /*[out,retval]*/ float * BaselineOffset ) = 0;
      virtual HRESULT __stdcall put_BaselineOffset (
        /*[in]*/ float BaselineOffset ) = 0;
      virtual HRESULT __stdcall get_Embedded (
        /*[out,retval]*/ enum Office2010::MsoTriState * Embedded ) = 0;
      virtual HRESULT __stdcall get_Embeddable (
        /*[out,retval]*/ enum Office2010::MsoTriState * Embeddable ) = 0;
      virtual HRESULT __stdcall get_Size (
        /*[out,retval]*/ float * Size ) = 0;
      virtual HRESULT __stdcall put_Size (
        /*[in]*/ float Size ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall put_Name (
        /*[in]*/ BSTR Name ) = 0;
      virtual HRESULT __stdcall get_NameFarEast (
        /*[out,retval]*/ BSTR * NameFarEast ) = 0;
      virtual HRESULT __stdcall put_NameFarEast (
        /*[in]*/ BSTR NameFarEast ) = 0;
      virtual HRESULT __stdcall get_NameAscii (
        /*[out,retval]*/ BSTR * NameAscii ) = 0;
      virtual HRESULT __stdcall put_NameAscii (
        /*[in]*/ BSTR NameAscii ) = 0;
      virtual HRESULT __stdcall get_AutoRotateNumbers (
        /*[out,retval]*/ enum Office2010::MsoTriState * AutoRotateNumbers ) = 0;
      virtual HRESULT __stdcall put_AutoRotateNumbers (
        /*[in]*/ enum Office2010::MsoTriState AutoRotateNumbers ) = 0;
      virtual HRESULT __stdcall get_NameOther (
        /*[out,retval]*/ BSTR * NameOther ) = 0;
      virtual HRESULT __stdcall put_NameOther (
        /*[in]*/ BSTR NameOther ) = 0;
      virtual HRESULT __stdcall get_NameComplexScript (
        /*[out,retval]*/ BSTR * NameComplexScript ) = 0;
      virtual HRESULT __stdcall put_NameComplexScript (
        /*[in]*/ BSTR NameComplexScript ) = 0;
};

struct __declspec(uuid("91493467-5a91-11cf-8700-00aa0060263b"))
Fonts : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct Font * * Item ) = 0;
      virtual HRESULT __stdcall Replace (
        /*[in]*/ BSTR Original,
        /*[in]*/ BSTR Replacement ) = 0;
};

struct __declspec(uuid("9149349c-5a91-11cf-8700-00aa0060263b"))
HeaderFooter : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Visible (
        /*[out,retval]*/ enum Office2010::MsoTriState * Visible ) = 0;
      virtual HRESULT __stdcall put_Visible (
        /*[in]*/ enum Office2010::MsoTriState Visible ) = 0;
      virtual HRESULT __stdcall get_Text (
        /*[out,retval]*/ BSTR * Text ) = 0;
      virtual HRESULT __stdcall put_Text (
        /*[in]*/ BSTR Text ) = 0;
      virtual HRESULT __stdcall get_UseFormat (
        /*[out,retval]*/ enum Office2010::MsoTriState * UseFormat ) = 0;
      virtual HRESULT __stdcall put_UseFormat (
        /*[in]*/ enum Office2010::MsoTriState UseFormat ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ enum PpDateTimeFormat * Format ) = 0;
      virtual HRESULT __stdcall put_Format (
        /*[in]*/ enum PpDateTimeFormat Format ) = 0;
};

struct __declspec(uuid("91493474-5a91-11cf-8700-00aa0060263b"))
HeadersFooters : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_DateAndTime (
        /*[out,retval]*/ struct HeaderFooter * * DateAndTime ) = 0;
      virtual HRESULT __stdcall get_SlideNumber (
        /*[out,retval]*/ struct HeaderFooter * * SlideNumber ) = 0;
      virtual HRESULT __stdcall get_Header (
        /*[out,retval]*/ struct HeaderFooter * * Header ) = 0;
      virtual HRESULT __stdcall get_Footer (
        /*[out,retval]*/ struct HeaderFooter * * Footer ) = 0;
      virtual HRESULT __stdcall get_DisplayOnTitleSlide (
        /*[out,retval]*/ enum Office2010::MsoTriState * DisplayOnTitleSlide ) = 0;
      virtual HRESULT __stdcall put_DisplayOnTitleSlide (
        /*[in]*/ enum Office2010::MsoTriState DisplayOnTitleSlide ) = 0;
      virtual HRESULT __stdcall Clear ( ) = 0;
};

struct __declspec(uuid("914934b9-5a91-11cf-8700-00aa0060263b"))
Tags : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ BSTR Name,
        /*[out,retval]*/ BSTR * Item ) = 0;
      virtual HRESULT __stdcall Add (
        /*[in]*/ BSTR Name,
        /*[in]*/ BSTR Value ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[in]*/ BSTR Name ) = 0;
      virtual HRESULT __stdcall AddBinary (
        /*[in]*/ BSTR Name,
        /*[in]*/ BSTR FilePath ) = 0;
      virtual HRESULT __stdcall BinaryValue (
        /*[in]*/ BSTR Name,
        /*[out,retval]*/ long * BinaryValue ) = 0;
      virtual HRESULT __stdcall Name (
        /*[in]*/ int Index,
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall Value (
        /*[in]*/ int Index,
        /*[out,retval]*/ BSTR * Value ) = 0;
};

struct __declspec(uuid("914934be-5a91-11cf-8700-00aa0060263b"))
MouseTracker : IUnknown
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall OnTrack (
        /*[in]*/ float X,
        /*[in]*/ float Y ) = 0;
      virtual HRESULT __stdcall EndTrack (
        /*[in]*/ float X,
        /*[in]*/ float Y ) = 0;
};

struct __declspec(uuid("914934bf-5a91-11cf-8700-00aa0060263b"))
MouseDownHandler : IUnknown
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall OnMouseDown (
        /*[in]*/ IUnknown * activeWin ) = 0;
};

struct __declspec(uuid("914934c0-5a91-11cf-8700-00aa0060263b"))
OCXExtender : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Visible (
        /*[out,retval]*/ VARIANT_BOOL * Visible ) = 0;
      virtual HRESULT __stdcall put_Visible (
        /*[in]*/ VARIANT_BOOL Visible ) = 0;
      virtual HRESULT __stdcall get_Left (
        /*[out,retval]*/ float * Left ) = 0;
      virtual HRESULT __stdcall put_Left (
        /*[in]*/ float Left ) = 0;
      virtual HRESULT __stdcall get_Top (
        /*[out,retval]*/ float * Top ) = 0;
      virtual HRESULT __stdcall put_Top (
        /*[in]*/ float Top ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ float * Height ) = 0;
      virtual HRESULT __stdcall put_Height (
        /*[in]*/ float Height ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ float * Width ) = 0;
      virtual HRESULT __stdcall put_Width (
        /*[in]*/ float Width ) = 0;
      virtual HRESULT __stdcall get_ZOrderPosition (
        /*[out,retval]*/ int * ZOrderPosition ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall put_Name (
        /*[in]*/ BSTR Name ) = 0;
      virtual HRESULT __stdcall get_AltHTML (
        /*[out,retval]*/ BSTR * AltHTML ) = 0;
      virtual HRESULT __stdcall put_AltHTML (
        /*[in]*/ BSTR AltHTML ) = 0;
};

struct __declspec(uuid("914934c1-5a91-11cf-8700-00aa0060263b"))
OCXExtenderEvents : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall GotFocus ( ) = 0;
      virtual HRESULT __stdcall LostFocus ( ) = 0;
};

struct __declspec(uuid("91493446-5a91-11cf-8700-00aa0060263b"))
OLEControl;
    // [ default ] interface OCXExtender
    // [ default, source ] interface OCXExtenderEvents

enum __declspec(uuid("16c96dbe-e683-4bab-9358-58c539857de2"))
PpBorderType
{
    ppBorderTop = 1,
    ppBorderLeft = 2,
    ppBorderBottom = 3,
    ppBorderRight = 4,
    ppBorderDiagonalDown = 5,
    ppBorderDiagonalUp = 6
};

enum __declspec(uuid("24814860-3221-485e-805a-9db3e4b55775"))
PpHTMLVersion
{
    ppHTMLv3 = 1,
    ppHTMLv4 = 2,
    ppHTMLDual = 3,
    ppHTMLAutodetect = 4
};

enum __declspec(uuid("da5cb2a4-456b-4906-b3fa-5191f98f7068"))
PpPublishSourceType
{
    ppPublishAll = 1,
    ppPublishSlideRange = 2,
    ppPublishNamedSlideShow = 3
};

enum __declspec(uuid("66b3a5be-c68b-42e2-8ef9-a5bd49af516b"))
PpBulletType
{
    ppBulletMixed = -2,
    ppBulletNone = 0,
    ppBulletUnnumbered = 1,
    ppBulletNumbered = 2,
    ppBulletPicture = 3
};

enum __declspec(uuid("63740092-ef1c-4097-8147-d3e7c7a0be98"))
PpNumberedBulletStyle
{
    ppBulletStyleMixed = -2,
    ppBulletAlphaLCPeriod = 0,
    ppBulletAlphaUCPeriod = 1,
    ppBulletArabicParenRight = 2,
    ppBulletArabicPeriod = 3,
    ppBulletRomanLCParenBoth = 4,
    ppBulletRomanLCParenRight = 5,
    ppBulletRomanLCPeriod = 6,
    ppBulletRomanUCPeriod = 7,
    ppBulletAlphaLCParenBoth = 8,
    ppBulletAlphaLCParenRight = 9,
    ppBulletAlphaUCParenBoth = 10,
    ppBulletAlphaUCParenRight = 11,
    ppBulletArabicParenBoth = 12,
    ppBulletArabicPlain = 13,
    ppBulletRomanUCParenBoth = 14,
    ppBulletRomanUCParenRight = 15,
    ppBulletSimpChinPlain = 16,
    ppBulletSimpChinPeriod = 17,
    ppBulletCircleNumDBPlain = 18,
    ppBulletCircleNumWDWhitePlain = 19,
    ppBulletCircleNumWDBlackPlain = 20,
    ppBulletTradChinPlain = 21,
    ppBulletTradChinPeriod = 22,
    ppBulletArabicAlphaDash = 23,
    ppBulletArabicAbjadDash = 24,
    ppBulletHebrewAlphaDash = 25,
    ppBulletKanjiKoreanPlain = 26,
    ppBulletKanjiKoreanPeriod = 27,
    ppBulletArabicDBPlain = 28,
    ppBulletArabicDBPeriod = 29,
    ppBulletThaiAlphaPeriod = 30,
    ppBulletThaiAlphaParenRight = 31,
    ppBulletThaiAlphaParenBoth = 32,
    ppBulletThaiNumPeriod = 33,
    ppBulletThaiNumParenRight = 34,
    ppBulletThaiNumParenBoth = 35,
    ppBulletHindiAlphaPeriod = 36,
    ppBulletHindiNumPeriod = 37,
    ppBulletKanjiSimpChinDBPeriod = 38,
    ppBulletHindiNumParenRight = 39,
    ppBulletHindiAlpha1Period = 40
};

struct __declspec(uuid("91493497-5a91-11cf-8700-00aa0060263b"))
BulletFormat : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Visible (
        /*[out,retval]*/ enum Office2010::MsoTriState * Visible ) = 0;
      virtual HRESULT __stdcall put_Visible (
        /*[in]*/ enum Office2010::MsoTriState Visible ) = 0;
      virtual HRESULT __stdcall get_Character (
        /*[out,retval]*/ int * Character ) = 0;
      virtual HRESULT __stdcall put_Character (
        /*[in]*/ int Character ) = 0;
      virtual HRESULT __stdcall get_RelativeSize (
        /*[out,retval]*/ float * RelativeSize ) = 0;
      virtual HRESULT __stdcall put_RelativeSize (
        /*[in]*/ float RelativeSize ) = 0;
      virtual HRESULT __stdcall get_UseTextColor (
        /*[out,retval]*/ enum Office2010::MsoTriState * UseTextColor ) = 0;
      virtual HRESULT __stdcall put_UseTextColor (
        /*[in]*/ enum Office2010::MsoTriState UseTextColor ) = 0;
      virtual HRESULT __stdcall get_UseTextFont (
        /*[out,retval]*/ enum Office2010::MsoTriState * UseTextFont ) = 0;
      virtual HRESULT __stdcall put_UseTextFont (
        /*[in]*/ enum Office2010::MsoTriState UseTextFont ) = 0;
      virtual HRESULT __stdcall get_Font (
        /*[out,retval]*/ struct Font * * Font ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum PpBulletType * Type ) = 0;
      virtual HRESULT __stdcall put_Type (
        /*[in]*/ enum PpBulletType Type ) = 0;
      virtual HRESULT __stdcall get_Style (
        /*[out,retval]*/ enum PpNumberedBulletStyle * Style ) = 0;
      virtual HRESULT __stdcall put_Style (
        /*[in]*/ enum PpNumberedBulletStyle Style ) = 0;
      virtual HRESULT __stdcall get_StartValue (
        /*[out,retval]*/ int * StartValue ) = 0;
      virtual HRESULT __stdcall put_StartValue (
        /*[in]*/ int StartValue ) = 0;
      virtual HRESULT __stdcall Picture (
        /*[in]*/ BSTR Picture ) = 0;
      virtual HRESULT __stdcall get_Number (
        /*[out,retval]*/ int * Number ) = 0;
};

struct __declspec(uuid("91493496-5a91-11cf-8700-00aa0060263b"))
ParagraphFormat : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Alignment (
        /*[out,retval]*/ enum PpParagraphAlignment * Alignment ) = 0;
      virtual HRESULT __stdcall put_Alignment (
        /*[in]*/ enum PpParagraphAlignment Alignment ) = 0;
      virtual HRESULT __stdcall get_Bullet (
        /*[out,retval]*/ struct BulletFormat * * Bullet ) = 0;
      virtual HRESULT __stdcall get_LineRuleBefore (
        /*[out,retval]*/ enum Office2010::MsoTriState * LineRuleBefore ) = 0;
      virtual HRESULT __stdcall put_LineRuleBefore (
        /*[in]*/ enum Office2010::MsoTriState LineRuleBefore ) = 0;
      virtual HRESULT __stdcall get_LineRuleAfter (
        /*[out,retval]*/ enum Office2010::MsoTriState * LineRuleAfter ) = 0;
      virtual HRESULT __stdcall put_LineRuleAfter (
        /*[in]*/ enum Office2010::MsoTriState LineRuleAfter ) = 0;
      virtual HRESULT __stdcall get_LineRuleWithin (
        /*[out,retval]*/ enum Office2010::MsoTriState * LineRuleWithin ) = 0;
      virtual HRESULT __stdcall put_LineRuleWithin (
        /*[in]*/ enum Office2010::MsoTriState LineRuleWithin ) = 0;
      virtual HRESULT __stdcall get_SpaceBefore (
        /*[out,retval]*/ float * SpaceBefore ) = 0;
      virtual HRESULT __stdcall put_SpaceBefore (
        /*[in]*/ float SpaceBefore ) = 0;
      virtual HRESULT __stdcall get_SpaceAfter (
        /*[out,retval]*/ float * SpaceAfter ) = 0;
      virtual HRESULT __stdcall put_SpaceAfter (
        /*[in]*/ float SpaceAfter ) = 0;
      virtual HRESULT __stdcall get_SpaceWithin (
        /*[out,retval]*/ float * SpaceWithin ) = 0;
      virtual HRESULT __stdcall put_SpaceWithin (
        /*[in]*/ float SpaceWithin ) = 0;
      virtual HRESULT __stdcall get_BaseLineAlignment (
        /*[out,retval]*/ enum PpBaselineAlignment * BaseLineAlignment ) = 0;
      virtual HRESULT __stdcall put_BaseLineAlignment (
        /*[in]*/ enum PpBaselineAlignment BaseLineAlignment ) = 0;
      virtual HRESULT __stdcall get_FarEastLineBreakControl (
        /*[out,retval]*/ enum Office2010::MsoTriState * FarEastLineBreakControl ) = 0;
      virtual HRESULT __stdcall put_FarEastLineBreakControl (
        /*[in]*/ enum Office2010::MsoTriState FarEastLineBreakControl ) = 0;
      virtual HRESULT __stdcall get_WordWrap (
        /*[out,retval]*/ enum Office2010::MsoTriState * WordWrap ) = 0;
      virtual HRESULT __stdcall put_WordWrap (
        /*[in]*/ enum Office2010::MsoTriState WordWrap ) = 0;
      virtual HRESULT __stdcall get_HangingPunctuation (
        /*[out,retval]*/ enum Office2010::MsoTriState * HangingPunctuation ) = 0;
      virtual HRESULT __stdcall put_HangingPunctuation (
        /*[in]*/ enum Office2010::MsoTriState HangingPunctuation ) = 0;
      virtual HRESULT __stdcall get_TextDirection (
        /*[out,retval]*/ enum PpDirection * TextDirection ) = 0;
      virtual HRESULT __stdcall put_TextDirection (
        /*[in]*/ enum PpDirection TextDirection ) = 0;
};

struct __declspec(uuid("9149349b-5a91-11cf-8700-00aa0060263b"))
TextStyleLevel : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_ParagraphFormat (
        /*[out,retval]*/ struct ParagraphFormat * * ParagraphFormat ) = 0;
      virtual HRESULT __stdcall get_Font (
        /*[out,retval]*/ struct Font * * Font ) = 0;
};

struct __declspec(uuid("9149349a-5a91-11cf-8700-00aa0060263b"))
TextStyleLevels : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Level,
        /*[out,retval]*/ struct TextStyleLevel * * Item ) = 0;
};

enum __declspec(uuid("8e3d8175-9840-4f3d-9558-adbcf2339e94"))
PpShapeFormat
{
    ppShapeFormatGIF = 0,
    ppShapeFormatJPG = 1,
    ppShapeFormatPNG = 2,
    ppShapeFormatBMP = 3,
    ppShapeFormatWMF = 4,
    ppShapeFormatEMF = 5
};

enum __declspec(uuid("9c5b7379-ce75-4980-8e9b-d0e957af4a41"))
PpExportMode
{
    ppRelativeToSlide = 1,
    ppClipRelativeToSlide = 2,
    ppScaleToFit = 3,
    ppScaleXY = 4
};

struct __declspec(uuid("914934ca-5a91-11cf-8700-00aa0060263b"))
Borders : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ enum PpBorderType BorderType,
        /*[out,retval]*/ struct LineFormat * * Item ) = 0;
};

struct __declspec(uuid("914934cc-5a91-11cf-8700-00aa0060263b"))
Pane : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Activate ( ) = 0;
      virtual HRESULT __stdcall get_Active (
        /*[out,retval]*/ enum Office2010::MsoTriState * Active ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_ViewType (
        /*[out,retval]*/ enum PpViewType * ViewType ) = 0;
};

struct __declspec(uuid("914934cb-5a91-11cf-8700-00aa0060263b"))
Panes : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct Pane * * Item ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
};

struct __declspec(uuid("914934cd-5a91-11cf-8700-00aa0060263b"))
DefaultWebOptions : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_IncludeNavigation (
        /*[out,retval]*/ enum Office2010::MsoTriState * IncludeNavigation ) = 0;
      virtual HRESULT __stdcall put_IncludeNavigation (
        /*[in]*/ enum Office2010::MsoTriState IncludeNavigation ) = 0;
      virtual HRESULT __stdcall get_FrameColors (
        /*[out,retval]*/ enum PpFrameColors * FrameColors ) = 0;
      virtual HRESULT __stdcall put_FrameColors (
        /*[in]*/ enum PpFrameColors FrameColors ) = 0;
      virtual HRESULT __stdcall get_ResizeGraphics (
        /*[out,retval]*/ enum Office2010::MsoTriState * ResizeGraphics ) = 0;
      virtual HRESULT __stdcall put_ResizeGraphics (
        /*[in]*/ enum Office2010::MsoTriState ResizeGraphics ) = 0;
      virtual HRESULT __stdcall get_ShowSlideAnimation (
        /*[out,retval]*/ enum Office2010::MsoTriState * ShowSlideAnimation ) = 0;
      virtual HRESULT __stdcall put_ShowSlideAnimation (
        /*[in]*/ enum Office2010::MsoTriState ShowSlideAnimation ) = 0;
      virtual HRESULT __stdcall get_OrganizeInFolder (
        /*[out,retval]*/ enum Office2010::MsoTriState * OrganizeInFolder ) = 0;
      virtual HRESULT __stdcall put_OrganizeInFolder (
        /*[in]*/ enum Office2010::MsoTriState OrganizeInFolder ) = 0;
      virtual HRESULT __stdcall get_UseLongFileNames (
        /*[out,retval]*/ enum Office2010::MsoTriState * UseLongFileNames ) = 0;
      virtual HRESULT __stdcall put_UseLongFileNames (
        /*[in]*/ enum Office2010::MsoTriState UseLongFileNames ) = 0;
      virtual HRESULT __stdcall get_RelyOnVML (
        /*[out,retval]*/ enum Office2010::MsoTriState * RelyOnVML ) = 0;
      virtual HRESULT __stdcall put_RelyOnVML (
        /*[in]*/ enum Office2010::MsoTriState RelyOnVML ) = 0;
      virtual HRESULT __stdcall get_AllowPNG (
        /*[out,retval]*/ enum Office2010::MsoTriState * AllowPNG ) = 0;
      virtual HRESULT __stdcall put_AllowPNG (
        /*[in]*/ enum Office2010::MsoTriState AllowPNG ) = 0;
      virtual HRESULT __stdcall get_ScreenSize (
        /*[out,retval]*/ enum Office2010::MsoScreenSize * ScreenSize ) = 0;
      virtual HRESULT __stdcall put_ScreenSize (
        /*[in]*/ enum Office2010::MsoScreenSize ScreenSize ) = 0;
      virtual HRESULT __stdcall get_Encoding (
        /*[out,retval]*/ enum Office2010::MsoEncoding * Encoding ) = 0;
      virtual HRESULT __stdcall put_Encoding (
        /*[in]*/ enum Office2010::MsoEncoding Encoding ) = 0;
      virtual HRESULT __stdcall get_UpdateLinksOnSave (
        /*[out,retval]*/ enum Office2010::MsoTriState * UpdateLinksOnSave ) = 0;
      virtual HRESULT __stdcall put_UpdateLinksOnSave (
        /*[in]*/ enum Office2010::MsoTriState UpdateLinksOnSave ) = 0;
      virtual HRESULT __stdcall get_CheckIfOfficeIsHTMLEditor (
        /*[out,retval]*/ enum Office2010::MsoTriState * CheckIfOfficeIsHTMLEditor ) = 0;
      virtual HRESULT __stdcall put_CheckIfOfficeIsHTMLEditor (
        /*[in]*/ enum Office2010::MsoTriState CheckIfOfficeIsHTMLEditor ) = 0;
      virtual HRESULT __stdcall get_AlwaysSaveInDefaultEncoding (
        /*[out,retval]*/ enum Office2010::MsoTriState * AlwaysSaveInDefaultEncoding ) = 0;
      virtual HRESULT __stdcall put_AlwaysSaveInDefaultEncoding (
        /*[in]*/ enum Office2010::MsoTriState AlwaysSaveInDefaultEncoding ) = 0;
      virtual HRESULT __stdcall get_Fonts (
        /*[out,retval]*/ struct Office2010::WebPageFonts * * Fonts ) = 0;
      virtual HRESULT __stdcall get_FolderSuffix (
        /*[out,retval]*/ BSTR * FolderSuffix ) = 0;
      virtual HRESULT __stdcall get_TargetBrowser (
        /*[out,retval]*/ enum Office2010::MsoTargetBrowser * TargetBrowser ) = 0;
      virtual HRESULT __stdcall put_TargetBrowser (
        /*[in]*/ enum Office2010::MsoTargetBrowser TargetBrowser ) = 0;
      virtual HRESULT __stdcall get_SaveNewWebPagesAsWebArchives (
        /*[out,retval]*/ enum Office2010::MsoTriState * SaveNewWebPagesAsWebArchives ) = 0;
      virtual HRESULT __stdcall put_SaveNewWebPagesAsWebArchives (
        /*[in]*/ enum Office2010::MsoTriState SaveNewWebPagesAsWebArchives ) = 0;
      virtual HRESULT __stdcall get_HTMLVersion (
        /*[out,retval]*/ enum PpHTMLVersion * HTMLVersion ) = 0;
      virtual HRESULT __stdcall put_HTMLVersion (
        /*[in]*/ enum PpHTMLVersion HTMLVersion ) = 0;
};

struct __declspec(uuid("914934ce-5a91-11cf-8700-00aa0060263b"))
WebOptions : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_IncludeNavigation (
        /*[out,retval]*/ enum Office2010::MsoTriState * IncludeNavigation ) = 0;
      virtual HRESULT __stdcall put_IncludeNavigation (
        /*[in]*/ enum Office2010::MsoTriState IncludeNavigation ) = 0;
      virtual HRESULT __stdcall get_FrameColors (
        /*[out,retval]*/ enum PpFrameColors * FrameColors ) = 0;
      virtual HRESULT __stdcall put_FrameColors (
        /*[in]*/ enum PpFrameColors FrameColors ) = 0;
      virtual HRESULT __stdcall get_ResizeGraphics (
        /*[out,retval]*/ enum Office2010::MsoTriState * ResizeGraphics ) = 0;
      virtual HRESULT __stdcall put_ResizeGraphics (
        /*[in]*/ enum Office2010::MsoTriState ResizeGraphics ) = 0;
      virtual HRESULT __stdcall get_ShowSlideAnimation (
        /*[out,retval]*/ enum Office2010::MsoTriState * ShowSlideAnimation ) = 0;
      virtual HRESULT __stdcall put_ShowSlideAnimation (
        /*[in]*/ enum Office2010::MsoTriState ShowSlideAnimation ) = 0;
      virtual HRESULT __stdcall get_OrganizeInFolder (
        /*[out,retval]*/ enum Office2010::MsoTriState * OrganizeInFolder ) = 0;
      virtual HRESULT __stdcall put_OrganizeInFolder (
        /*[in]*/ enum Office2010::MsoTriState OrganizeInFolder ) = 0;
      virtual HRESULT __stdcall get_UseLongFileNames (
        /*[out,retval]*/ enum Office2010::MsoTriState * UseLongFileNames ) = 0;
      virtual HRESULT __stdcall put_UseLongFileNames (
        /*[in]*/ enum Office2010::MsoTriState UseLongFileNames ) = 0;
      virtual HRESULT __stdcall get_RelyOnVML (
        /*[out,retval]*/ enum Office2010::MsoTriState * RelyOnVML ) = 0;
      virtual HRESULT __stdcall put_RelyOnVML (
        /*[in]*/ enum Office2010::MsoTriState RelyOnVML ) = 0;
      virtual HRESULT __stdcall get_AllowPNG (
        /*[out,retval]*/ enum Office2010::MsoTriState * AllowPNG ) = 0;
      virtual HRESULT __stdcall put_AllowPNG (
        /*[in]*/ enum Office2010::MsoTriState AllowPNG ) = 0;
      virtual HRESULT __stdcall get_ScreenSize (
        /*[out,retval]*/ enum Office2010::MsoScreenSize * ScreenSize ) = 0;
      virtual HRESULT __stdcall put_ScreenSize (
        /*[in]*/ enum Office2010::MsoScreenSize ScreenSize ) = 0;
      virtual HRESULT __stdcall get_Encoding (
        /*[out,retval]*/ enum Office2010::MsoEncoding * Encoding ) = 0;
      virtual HRESULT __stdcall put_Encoding (
        /*[in]*/ enum Office2010::MsoEncoding Encoding ) = 0;
      virtual HRESULT __stdcall get_FolderSuffix (
        /*[out,retval]*/ BSTR * FolderSuffix ) = 0;
      virtual HRESULT __stdcall UseDefaultFolderSuffix ( ) = 0;
      virtual HRESULT __stdcall get_TargetBrowser (
        /*[out,retval]*/ enum Office2010::MsoTargetBrowser * TargetBrowser ) = 0;
      virtual HRESULT __stdcall put_TargetBrowser (
        /*[in]*/ enum Office2010::MsoTargetBrowser TargetBrowser ) = 0;
      virtual HRESULT __stdcall get_HTMLVersion (
        /*[out,retval]*/ enum PpHTMLVersion * HTMLVersion ) = 0;
      virtual HRESULT __stdcall put_HTMLVersion (
        /*[in]*/ enum PpHTMLVersion HTMLVersion ) = 0;
};

struct __declspec(uuid("914934d0-5a91-11cf-8700-00aa0060263b"))
PublishObject : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_HTMLVersion (
        /*[out,retval]*/ enum PpHTMLVersion * HTMLVersion ) = 0;
      virtual HRESULT __stdcall put_HTMLVersion (
        /*[in]*/ enum PpHTMLVersion HTMLVersion ) = 0;
      virtual HRESULT __stdcall get_SourceType (
        /*[out,retval]*/ enum PpPublishSourceType * SourceType ) = 0;
      virtual HRESULT __stdcall put_SourceType (
        /*[in]*/ enum PpPublishSourceType SourceType ) = 0;
      virtual HRESULT __stdcall get_RangeStart (
        /*[out,retval]*/ int * RangeStart ) = 0;
      virtual HRESULT __stdcall put_RangeStart (
        /*[in]*/ int RangeStart ) = 0;
      virtual HRESULT __stdcall get_RangeEnd (
        /*[out,retval]*/ int * RangeEnd ) = 0;
      virtual HRESULT __stdcall put_RangeEnd (
        /*[in]*/ int RangeEnd ) = 0;
      virtual HRESULT __stdcall get_SlideShowName (
        /*[out,retval]*/ BSTR * SlideShowName ) = 0;
      virtual HRESULT __stdcall put_SlideShowName (
        /*[in]*/ BSTR SlideShowName ) = 0;
      virtual HRESULT __stdcall get_SpeakerNotes (
        /*[out,retval]*/ enum Office2010::MsoTriState * SpeakerNotes ) = 0;
      virtual HRESULT __stdcall put_SpeakerNotes (
        /*[in]*/ enum Office2010::MsoTriState SpeakerNotes ) = 0;
      virtual HRESULT __stdcall get_FileName (
        /*[out,retval]*/ BSTR * FileName ) = 0;
      virtual HRESULT __stdcall put_FileName (
        /*[in]*/ BSTR FileName ) = 0;
      virtual HRESULT __stdcall Publish ( ) = 0;
};

struct __declspec(uuid("914934cf-5a91-11cf-8700-00aa0060263b"))
PublishObjects : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct PublishObject * * Item ) = 0;
};

struct __declspec(uuid("914934d2-5a91-11cf-8700-00aa0060263b"))
MasterEvents : IUnknown
{};

struct __declspec(uuid("91493447-5a91-11cf-8700-00aa0060263b"))
Master;
    // [ default ] interface _Master
    // [ default, source ] interface MasterEvents

enum __declspec(uuid("83329628-509c-4cf8-a6b4-afca95a071bb"))
PpPasteDataType
{
    ppPasteDefault = 0,
    ppPasteBitmap = 1,
    ppPasteEnhancedMetafile = 2,
    ppPasteMetafilePicture = 3,
    ppPasteGIF = 4,
    ppPasteJPG = 5,
    ppPastePNG = 6,
    ppPasteText = 7,
    ppPasteHTML = 8,
    ppPasteRTF = 9,
    ppPasteOLEObject = 10,
    ppPasteShape = 11
};

struct __declspec(uuid("9149348f-5a91-11cf-8700-00aa0060263b"))
TextRange : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_ActionSettings (
        /*[out,retval]*/ struct ActionSettings * * ActionSettings ) = 0;
      virtual HRESULT __stdcall get_Start (
        /*[out,retval]*/ long * Start ) = 0;
      virtual HRESULT __stdcall get_Length (
        /*[out,retval]*/ long * Length ) = 0;
      virtual HRESULT __stdcall get_BoundLeft (
        /*[out,retval]*/ float * BoundLeft ) = 0;
      virtual HRESULT __stdcall get_BoundTop (
        /*[out,retval]*/ float * BoundTop ) = 0;
      virtual HRESULT __stdcall get_BoundWidth (
        /*[out,retval]*/ float * BoundWidth ) = 0;
      virtual HRESULT __stdcall get_BoundHeight (
        /*[out,retval]*/ float * BoundHeight ) = 0;
      virtual HRESULT __stdcall Paragraphs (
        /*[in]*/ int Start,
        /*[in]*/ int Length,
        /*[out,retval]*/ struct TextRange * * Paragraphs ) = 0;
      virtual HRESULT __stdcall Sentences (
        /*[in]*/ int Start,
        /*[in]*/ int Length,
        /*[out,retval]*/ struct TextRange * * Sentences ) = 0;
      virtual HRESULT __stdcall Words (
        /*[in]*/ int Start,
        /*[in]*/ int Length,
        /*[out,retval]*/ struct TextRange * * Words ) = 0;
      virtual HRESULT __stdcall Characters (
        /*[in]*/ int Start,
        /*[in]*/ int Length,
        /*[out,retval]*/ struct TextRange * * Characters ) = 0;
      virtual HRESULT __stdcall Lines (
        /*[in]*/ int Start,
        /*[in]*/ int Length,
        /*[out,retval]*/ struct TextRange * * Lines ) = 0;
      virtual HRESULT __stdcall Runs (
        /*[in]*/ int Start,
        /*[in]*/ int Length,
        /*[out,retval]*/ struct TextRange * * Runs ) = 0;
      virtual HRESULT __stdcall TrimText (
        /*[out,retval]*/ struct TextRange * * TrimText ) = 0;
      virtual HRESULT __stdcall get_Text (
        /*[out,retval]*/ BSTR * Text ) = 0;
      virtual HRESULT __stdcall put_Text (
        /*[in]*/ BSTR Text ) = 0;
      virtual HRESULT __stdcall InsertAfter (
        /*[in]*/ BSTR NewText,
        /*[out,retval]*/ struct TextRange * * After ) = 0;
      virtual HRESULT __stdcall InsertBefore (
        /*[in]*/ BSTR NewText,
        /*[out,retval]*/ struct TextRange * * Before ) = 0;
      virtual HRESULT __stdcall InsertDateTime (
        /*[in]*/ enum PpDateTimeFormat DateTimeFormat,
        /*[in]*/ enum Office2010::MsoTriState InsertAsField,
        /*[out,retval]*/ struct TextRange * * DateTime ) = 0;
      virtual HRESULT __stdcall InsertSlideNumber (
        /*[out,retval]*/ struct TextRange * * SlideNumber ) = 0;
      virtual HRESULT __stdcall InsertSymbol (
        /*[in]*/ BSTR FontName,
        /*[in]*/ int CharNumber,
        /*[in]*/ enum Office2010::MsoTriState Unicode,
        /*[out,retval]*/ struct TextRange * * Symbol ) = 0;
      virtual HRESULT __stdcall get_Font (
        /*[out,retval]*/ struct Font * * Font ) = 0;
      virtual HRESULT __stdcall get_ParagraphFormat (
        /*[out,retval]*/ struct ParagraphFormat * * ParagraphFormat ) = 0;
      virtual HRESULT __stdcall get_IndentLevel (
        /*[out,retval]*/ int * IndentLevel ) = 0;
      virtual HRESULT __stdcall put_IndentLevel (
        /*[in]*/ int IndentLevel ) = 0;
      virtual HRESULT __stdcall Select ( ) = 0;
      virtual HRESULT __stdcall Cut ( ) = 0;
      virtual HRESULT __stdcall Copy ( ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
      virtual HRESULT __stdcall Paste (
        /*[out,retval]*/ struct TextRange * * Paste ) = 0;
      virtual HRESULT __stdcall ChangeCase (
        /*[in]*/ enum PpChangeCase Type ) = 0;
      virtual HRESULT __stdcall AddPeriods ( ) = 0;
      virtual HRESULT __stdcall RemovePeriods ( ) = 0;
      virtual HRESULT __stdcall Find (
        /*[in]*/ BSTR FindWhat,
        /*[in]*/ int After,
        /*[in]*/ enum Office2010::MsoTriState MatchCase,
        /*[in]*/ enum Office2010::MsoTriState WholeWords,
        /*[out,retval]*/ struct TextRange * * Find ) = 0;
      virtual HRESULT __stdcall Replace (
        /*[in]*/ BSTR FindWhat,
        /*[in]*/ BSTR ReplaceWhat,
        /*[in]*/ int After,
        /*[in]*/ enum Office2010::MsoTriState MatchCase,
        /*[in]*/ enum Office2010::MsoTriState WholeWords,
        /*[out,retval]*/ struct TextRange * * Replace ) = 0;
      virtual HRESULT __stdcall RotatedBounds (
        /*[out]*/ float * X1,
        /*[out]*/ float * Y1,
        /*[out]*/ float * X2,
        /*[out]*/ float * Y2,
        /*[out]*/ float * X3,
        /*[out]*/ float * Y3,
        /*[out]*/ float * x4,
        /*[out]*/ float * y4 ) = 0;
      virtual HRESULT __stdcall get_LanguageID (
        /*[out,retval]*/ enum Office2010::MsoLanguageID * LanguageID ) = 0;
      virtual HRESULT __stdcall put_LanguageID (
        /*[in]*/ enum Office2010::MsoLanguageID LanguageID ) = 0;
      virtual HRESULT __stdcall RtlRun ( ) = 0;
      virtual HRESULT __stdcall LtrRun ( ) = 0;
      virtual HRESULT __stdcall PasteSpecial (
        /*[in]*/ enum PpPasteDataType DataType,
        /*[in]*/ enum Office2010::MsoTriState DisplayAsIcon,
        /*[in]*/ BSTR IconFileName,
        /*[in]*/ int IconIndex,
        /*[in]*/ BSTR IconLabel,
        /*[in]*/ enum Office2010::MsoTriState Link,
        /*[out,retval]*/ struct TextRange * * PasteSpecial ) = 0;
};

struct __declspec(uuid("91493484-5a91-11cf-8700-00aa0060263b"))
TextFrame : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_MarginBottom (
        /*[out,retval]*/ float * MarginBottom ) = 0;
      virtual HRESULT __stdcall put_MarginBottom (
        /*[in]*/ float MarginBottom ) = 0;
      virtual HRESULT __stdcall get_MarginLeft (
        /*[out,retval]*/ float * MarginLeft ) = 0;
      virtual HRESULT __stdcall put_MarginLeft (
        /*[in]*/ float MarginLeft ) = 0;
      virtual HRESULT __stdcall get_MarginRight (
        /*[out,retval]*/ float * MarginRight ) = 0;
      virtual HRESULT __stdcall put_MarginRight (
        /*[in]*/ float MarginRight ) = 0;
      virtual HRESULT __stdcall get_MarginTop (
        /*[out,retval]*/ float * MarginTop ) = 0;
      virtual HRESULT __stdcall put_MarginTop (
        /*[in]*/ float MarginTop ) = 0;
      virtual HRESULT __stdcall get_Orientation (
        /*[out,retval]*/ enum Office2010::MsoTextOrientation * Orientation ) = 0;
      virtual HRESULT __stdcall put_Orientation (
        /*[in]*/ enum Office2010::MsoTextOrientation Orientation ) = 0;
      virtual HRESULT __stdcall get_HasText (
        /*[out,retval]*/ enum Office2010::MsoTriState * HasText ) = 0;
      virtual HRESULT __stdcall get_TextRange (
        /*[out,retval]*/ struct TextRange * * TextRange ) = 0;
      virtual HRESULT __stdcall get_Ruler (
        /*[out,retval]*/ struct Ruler * * Ruler ) = 0;
      virtual HRESULT __stdcall get_HorizontalAnchor (
        /*[out,retval]*/ enum Office2010::MsoHorizontalAnchor * HorizontalAnchor ) = 0;
      virtual HRESULT __stdcall put_HorizontalAnchor (
        /*[in]*/ enum Office2010::MsoHorizontalAnchor HorizontalAnchor ) = 0;
      virtual HRESULT __stdcall get_VerticalAnchor (
        /*[out,retval]*/ enum Office2010::MsoVerticalAnchor * VerticalAnchor ) = 0;
      virtual HRESULT __stdcall put_VerticalAnchor (
        /*[in]*/ enum Office2010::MsoVerticalAnchor VerticalAnchor ) = 0;
      virtual HRESULT __stdcall get_AutoSize (
        /*[out,retval]*/ enum PpAutoSize * AutoSize ) = 0;
      virtual HRESULT __stdcall put_AutoSize (
        /*[in]*/ enum PpAutoSize AutoSize ) = 0;
      virtual HRESULT __stdcall get_WordWrap (
        /*[out,retval]*/ enum Office2010::MsoTriState * WordWrap ) = 0;
      virtual HRESULT __stdcall put_WordWrap (
        /*[in]*/ enum Office2010::MsoTriState WordWrap ) = 0;
      virtual HRESULT __stdcall DeleteText ( ) = 0;
};

struct __declspec(uuid("91493499-5a91-11cf-8700-00aa0060263b"))
TextStyle : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Ruler (
        /*[out,retval]*/ struct Ruler * * Ruler ) = 0;
      virtual HRESULT __stdcall get_TextFrame (
        /*[out,retval]*/ struct TextFrame * * TextFrame ) = 0;
      virtual HRESULT __stdcall get_Levels (
        /*[out,retval]*/ struct TextStyleLevels * * Levels ) = 0;
};

struct __declspec(uuid("91493498-5a91-11cf-8700-00aa0060263b"))
TextStyles : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ enum PpTextStyleType Type,
        /*[out,retval]*/ struct TextStyle * * Item ) = 0;
};

struct __declspec(uuid("914934d3-5a91-11cf-8700-00aa0060263b"))
_PowerRex : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall OnAsfEncoderEvent (
        /*[in]*/ VARIANT erorCode,
        /*[in]*/ VARIANT bstrErrorDesc ) = 0;
};

struct __declspec(uuid("91493448-5a91-11cf-8700-00aa0060263b"))
PowerRex;
    // [ default ] interface _PowerRex

struct __declspec(uuid("914934d5-5a91-11cf-8700-00aa0060263b"))
Comment : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Author (
        /*[out,retval]*/ BSTR * Author ) = 0;
      virtual HRESULT __stdcall get_AuthorInitials (
        /*[out,retval]*/ BSTR * AuthorInitials ) = 0;
      virtual HRESULT __stdcall get_Text (
        /*[out,retval]*/ BSTR * Text ) = 0;
      virtual HRESULT __stdcall get_DateTime (
        /*[out,retval]*/ DATE * DateTime ) = 0;
      virtual HRESULT __stdcall get_AuthorIndex (
        /*[out,retval]*/ int * AuthorIndex ) = 0;
      virtual HRESULT __stdcall get_Left (
        /*[out,retval]*/ float * Left ) = 0;
      virtual HRESULT __stdcall get_Top (
        /*[out,retval]*/ float * Top ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
};

struct __declspec(uuid("914934d4-5a91-11cf-8700-00aa0060263b"))
Comments : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct Comment * * Item ) = 0;
      virtual HRESULT __stdcall Add (
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ BSTR Author,
        /*[in]*/ BSTR AuthorInitials,
        /*[in]*/ BSTR Text,
        /*[out,retval]*/ struct Comment * * Add ) = 0;
};

enum __declspec(uuid("4927ea82-23fb-4f6f-9c8b-4204ceb23d21"))
MsoAnimEffect
{
    msoAnimEffectCustom = 0,
    msoAnimEffectAppear = 1,
    msoAnimEffectFly = 2,
    msoAnimEffectBlinds = 3,
    msoAnimEffectBox = 4,
    msoAnimEffectCheckerboard = 5,
    msoAnimEffectCircle = 6,
    msoAnimEffectCrawl = 7,
    msoAnimEffectDiamond = 8,
    msoAnimEffectDissolve = 9,
    msoAnimEffectFade = 10,
    msoAnimEffectFlashOnce = 11,
    msoAnimEffectPeek = 12,
    msoAnimEffectPlus = 13,
    msoAnimEffectRandomBars = 14,
    msoAnimEffectSpiral = 15,
    msoAnimEffectSplit = 16,
    msoAnimEffectStretch = 17,
    msoAnimEffectStrips = 18,
    msoAnimEffectSwivel = 19,
    msoAnimEffectWedge = 20,
    msoAnimEffectWheel = 21,
    msoAnimEffectWipe = 22,
    msoAnimEffectZoom = 23,
    msoAnimEffectRandomEffects = 24,
    msoAnimEffectBoomerang = 25,
    msoAnimEffectBounce = 26,
    msoAnimEffectColorReveal = 27,
    msoAnimEffectCredits = 28,
    msoAnimEffectEaseIn = 29,
    msoAnimEffectFloat = 30,
    msoAnimEffectGrowAndTurn = 31,
    msoAnimEffectLightSpeed = 32,
    msoAnimEffectPinwheel = 33,
    msoAnimEffectRiseUp = 34,
    msoAnimEffectSwish = 35,
    msoAnimEffectThinLine = 36,
    msoAnimEffectUnfold = 37,
    msoAnimEffectWhip = 38,
    msoAnimEffectAscend = 39,
    msoAnimEffectCenterRevolve = 40,
    msoAnimEffectFadedSwivel = 41,
    msoAnimEffectDescend = 42,
    msoAnimEffectSling = 43,
    msoAnimEffectSpinner = 44,
    msoAnimEffectStretchy = 45,
    msoAnimEffectZip = 46,
    msoAnimEffectArcUp = 47,
    msoAnimEffectFadedZoom = 48,
    msoAnimEffectGlide = 49,
    msoAnimEffectExpand = 50,
    msoAnimEffectFlip = 51,
    msoAnimEffectShimmer = 52,
    msoAnimEffectFold = 53,
    msoAnimEffectChangeFillColor = 54,
    msoAnimEffectChangeFont = 55,
    msoAnimEffectChangeFontColor = 56,
    msoAnimEffectChangeFontSize = 57,
    msoAnimEffectChangeFontStyle = 58,
    msoAnimEffectGrowShrink = 59,
    msoAnimEffectChangeLineColor = 60,
    msoAnimEffectSpin = 61,
    msoAnimEffectTransparency = 62,
    msoAnimEffectBoldFlash = 63,
    msoAnimEffectBlast = 64,
    msoAnimEffectBoldReveal = 65,
    msoAnimEffectBrushOnColor = 66,
    msoAnimEffectBrushOnUnderline = 67,
    msoAnimEffectColorBlend = 68,
    msoAnimEffectColorWave = 69,
    msoAnimEffectComplementaryColor = 70,
    msoAnimEffectComplementaryColor2 = 71,
    msoAnimEffectContrastingColor = 72,
    msoAnimEffectDarken = 73,
    msoAnimEffectDesaturate = 74,
    msoAnimEffectFlashBulb = 75,
    msoAnimEffectFlicker = 76,
    msoAnimEffectGrowWithColor = 77,
    msoAnimEffectLighten = 78,
    msoAnimEffectStyleEmphasis = 79,
    msoAnimEffectTeeter = 80,
    msoAnimEffectVerticalGrow = 81,
    msoAnimEffectWave = 82,
    msoAnimEffectMediaPlay = 83,
    msoAnimEffectMediaPause = 84,
    msoAnimEffectMediaStop = 85,
    msoAnimEffectPathCircle = 86,
    msoAnimEffectPathRightTriangle = 87,
    msoAnimEffectPathDiamond = 88,
    msoAnimEffectPathHexagon = 89,
    msoAnimEffectPath5PointStar = 90,
    msoAnimEffectPathCrescentMoon = 91,
    msoAnimEffectPathSquare = 92,
    msoAnimEffectPathTrapezoid = 93,
    msoAnimEffectPathHeart = 94,
    msoAnimEffectPathOctagon = 95,
    msoAnimEffectPath6PointStar = 96,
    msoAnimEffectPathFootball = 97,
    msoAnimEffectPathEqualTriangle = 98,
    msoAnimEffectPathParallelogram = 99,
    msoAnimEffectPathPentagon = 100,
    msoAnimEffectPath4PointStar = 101,
    msoAnimEffectPath8PointStar = 102,
    msoAnimEffectPathTeardrop = 103,
    msoAnimEffectPathPointyStar = 104,
    msoAnimEffectPathCurvedSquare = 105,
    msoAnimEffectPathCurvedX = 106,
    msoAnimEffectPathVerticalFigure8 = 107,
    msoAnimEffectPathCurvyStar = 108,
    msoAnimEffectPathLoopdeLoop = 109,
    msoAnimEffectPathBuzzsaw = 110,
    msoAnimEffectPathHorizontalFigure8 = 111,
    msoAnimEffectPathPeanut = 112,
    msoAnimEffectPathFigure8Four = 113,
    msoAnimEffectPathNeutron = 114,
    msoAnimEffectPathSwoosh = 115,
    msoAnimEffectPathBean = 116,
    msoAnimEffectPathPlus = 117,
    msoAnimEffectPathInvertedTriangle = 118,
    msoAnimEffectPathInvertedSquare = 119,
    msoAnimEffectPathLeft = 120,
    msoAnimEffectPathTurnRight = 121,
    msoAnimEffectPathArcDown = 122,
    msoAnimEffectPathZigzag = 123,
    msoAnimEffectPathSCurve2 = 124,
    msoAnimEffectPathSineWave = 125,
    msoAnimEffectPathBounceLeft = 126,
    msoAnimEffectPathDown = 127,
    msoAnimEffectPathTurnUp = 128,
    msoAnimEffectPathArcUp = 129,
    msoAnimEffectPathHeartbeat = 130,
    msoAnimEffectPathSpiralRight = 131,
    msoAnimEffectPathWave = 132,
    msoAnimEffectPathCurvyLeft = 133,
    msoAnimEffectPathDiagonalDownRight = 134,
    msoAnimEffectPathTurnDown = 135,
    msoAnimEffectPathArcLeft = 136,
    msoAnimEffectPathFunnel = 137,
    msoAnimEffectPathSpring = 138,
    msoAnimEffectPathBounceRight = 139,
    msoAnimEffectPathSpiralLeft = 140,
    msoAnimEffectPathDiagonalUpRight = 141,
    msoAnimEffectPathTurnUpRight = 142,
    msoAnimEffectPathArcRight = 143,
    msoAnimEffectPathSCurve1 = 144,
    msoAnimEffectPathDecayingWave = 145,
    msoAnimEffectPathCurvyRight = 146,
    msoAnimEffectPathStairsDown = 147,
    msoAnimEffectPathUp = 148,
    msoAnimEffectPathRight = 149,
    msoAnimEffectMediaPlayFromBookmark = 150
};

enum __declspec(uuid("4290d45b-433a-452f-8402-447fd15187e0"))
MsoAnimateByLevel
{
    msoAnimateLevelMixed = -1,
    msoAnimateLevelNone = 0,
    msoAnimateTextByAllLevels = 1,
    msoAnimateTextByFirstLevel = 2,
    msoAnimateTextBySecondLevel = 3,
    msoAnimateTextByThirdLevel = 4,
    msoAnimateTextByFourthLevel = 5,
    msoAnimateTextByFifthLevel = 6,
    msoAnimateChartAllAtOnce = 7,
    msoAnimateChartByCategory = 8,
    msoAnimateChartByCategoryElements = 9,
    msoAnimateChartBySeries = 10,
    msoAnimateChartBySeriesElements = 11,
    msoAnimateDiagramAllAtOnce = 12,
    msoAnimateDiagramDepthByNode = 13,
    msoAnimateDiagramDepthByBranch = 14,
    msoAnimateDiagramBreadthByNode = 15,
    msoAnimateDiagramBreadthByLevel = 16,
    msoAnimateDiagramClockwise = 17,
    msoAnimateDiagramClockwiseIn = 18,
    msoAnimateDiagramClockwiseOut = 19,
    msoAnimateDiagramCounterClockwise = 20,
    msoAnimateDiagramCounterClockwiseIn = 21,
    msoAnimateDiagramCounterClockwiseOut = 22,
    msoAnimateDiagramInByRing = 23,
    msoAnimateDiagramOutByRing = 24,
    msoAnimateDiagramUp = 25,
    msoAnimateDiagramDown = 26
};

enum __declspec(uuid("45f2ed98-f126-47ef-9521-420dd5d364af"))
MsoAnimTriggerType
{
    msoAnimTriggerMixed = -1,
    msoAnimTriggerNone = 0,
    msoAnimTriggerOnPageClick = 1,
    msoAnimTriggerWithPrevious = 2,
    msoAnimTriggerAfterPrevious = 3,
    msoAnimTriggerOnShapeClick = 4,
    msoAnimTriggerOnMediaBookmark = 5
};

enum __declspec(uuid("6bf95896-a5b8-44c5-81e1-03d78481c890"))
MsoAnimAfterEffect
{
    msoAnimAfterEffectMixed = -1,
    msoAnimAfterEffectNone = 0,
    msoAnimAfterEffectDim = 1,
    msoAnimAfterEffectHide = 2,
    msoAnimAfterEffectHideOnNextClick = 3
};

enum __declspec(uuid("8ba1e4a7-c122-4400-af6b-6905a8863766"))
MsoAnimTextUnitEffect
{
    msoAnimTextUnitEffectMixed = -1,
    msoAnimTextUnitEffectByParagraph = 0,
    msoAnimTextUnitEffectByCharacter = 1,
    msoAnimTextUnitEffectByWord = 2
};

enum __declspec(uuid("da113e67-ed91-4427-9415-83960759d3fb"))
MsoAnimEffectRestart
{
    msoAnimEffectRestartAlways = 1,
    msoAnimEffectRestartWhenOff = 2,
    msoAnimEffectRestartNever = 3
};

enum __declspec(uuid("a2f1daf6-7eec-46c9-ab9f-877c909cb47d"))
MsoAnimEffectAfter
{
    msoAnimEffectAfterFreeze = 1,
    msoAnimEffectAfterRemove = 2,
    msoAnimEffectAfterHold = 3,
    msoAnimEffectAfterTransition = 4
};

enum __declspec(uuid("9d777915-eb83-4f30-aae5-d96db9a3cf7a"))
MsoAnimDirection
{
    msoAnimDirectionNone = 0,
    msoAnimDirectionUp = 1,
    msoAnimDirectionRight = 2,
    msoAnimDirectionDown = 3,
    msoAnimDirectionLeft = 4,
    msoAnimDirectionOrdinalMask = 5,
    msoAnimDirectionUpLeft = 6,
    msoAnimDirectionUpRight = 7,
    msoAnimDirectionDownRight = 8,
    msoAnimDirectionDownLeft = 9,
    msoAnimDirectionTop = 10,
    msoAnimDirectionBottom = 11,
    msoAnimDirectionTopLeft = 12,
    msoAnimDirectionTopRight = 13,
    msoAnimDirectionBottomRight = 14,
    msoAnimDirectionBottomLeft = 15,
    msoAnimDirectionHorizontal = 16,
    msoAnimDirectionVertical = 17,
    msoAnimDirectionAcross = 18,
    msoAnimDirectionIn = 19,
    msoAnimDirectionOut = 20,
    msoAnimDirectionClockwise = 21,
    msoAnimDirectionCounterclockwise = 22,
    msoAnimDirectionHorizontalIn = 23,
    msoAnimDirectionHorizontalOut = 24,
    msoAnimDirectionVerticalIn = 25,
    msoAnimDirectionVerticalOut = 26,
    msoAnimDirectionSlightly = 27,
    msoAnimDirectionCenter = 28,
    msoAnimDirectionInSlightly = 29,
    msoAnimDirectionInCenter = 30,
    msoAnimDirectionInBottom = 31,
    msoAnimDirectionOutSlightly = 32,
    msoAnimDirectionOutCenter = 33,
    msoAnimDirectionOutBottom = 34,
    msoAnimDirectionFontBold = 35,
    msoAnimDirectionFontItalic = 36,
    msoAnimDirectionFontUnderline = 37,
    msoAnimDirectionFontStrikethrough = 38,
    msoAnimDirectionFontShadow = 39,
    msoAnimDirectionFontAllCaps = 40,
    msoAnimDirectionInstant = 41,
    msoAnimDirectionGradual = 42,
    msoAnimDirectionCycleClockwise = 43,
    msoAnimDirectionCycleCounterclockwise = 44
};

struct __declspec(uuid("914934e1-5a91-11cf-8700-00aa0060263b"))
EffectParameters : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Direction (
        /*[out,retval]*/ enum MsoAnimDirection * Direction ) = 0;
      virtual HRESULT __stdcall put_Direction (
        /*[in]*/ enum MsoAnimDirection Direction ) = 0;
      virtual HRESULT __stdcall get_Amount (
        /*[out,retval]*/ float * Amount ) = 0;
      virtual HRESULT __stdcall put_Amount (
        /*[in]*/ float Amount ) = 0;
      virtual HRESULT __stdcall get_Size (
        /*[out,retval]*/ float * Size ) = 0;
      virtual HRESULT __stdcall put_Size (
        /*[in]*/ float Size ) = 0;
      virtual HRESULT __stdcall get_Color2 (
        /*[out,retval]*/ struct ColorFormat * * Color2 ) = 0;
      virtual HRESULT __stdcall get_Relative (
        /*[out,retval]*/ enum Office2010::MsoTriState * Relative ) = 0;
      virtual HRESULT __stdcall put_Relative (
        /*[in]*/ enum Office2010::MsoTriState Relative ) = 0;
      virtual HRESULT __stdcall get_FontName (
        /*[out,retval]*/ BSTR * FontName ) = 0;
      virtual HRESULT __stdcall put_FontName (
        /*[in]*/ BSTR FontName ) = 0;
};

struct __declspec(uuid("914934e2-5a91-11cf-8700-00aa0060263b"))
EffectInformation : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_AfterEffect (
        /*[out,retval]*/ enum MsoAnimAfterEffect * AfterEffect ) = 0;
      virtual HRESULT __stdcall get_AnimateBackground (
        /*[out,retval]*/ enum Office2010::MsoTriState * AnimateBackground ) = 0;
      virtual HRESULT __stdcall get_AnimateTextInReverse (
        /*[out,retval]*/ enum Office2010::MsoTriState * AnimateTextInReverse ) = 0;
      virtual HRESULT __stdcall get_BuildByLevelEffect (
        /*[out,retval]*/ enum MsoAnimateByLevel * BuildByLevelEffect ) = 0;
      virtual HRESULT __stdcall get_Dim (
        /*[out,retval]*/ struct ColorFormat * * Dim ) = 0;
      virtual HRESULT __stdcall get_PlaySettings (
        /*[out,retval]*/ struct PlaySettings * * PlaySettings ) = 0;
      virtual HRESULT __stdcall get_SoundEffect (
        /*[out,retval]*/ struct SoundEffect * * SoundEffect ) = 0;
      virtual HRESULT __stdcall get_TextUnitEffect (
        /*[out,retval]*/ enum MsoAnimTextUnitEffect * TextUnitEffect ) = 0;
};

enum __declspec(uuid("6b9efdd2-199b-41a0-8192-6a50cd6e521f"))
MsoAnimType
{
    msoAnimTypeMixed = -2,
    msoAnimTypeNone = 0,
    msoAnimTypeMotion = 1,
    msoAnimTypeColor = 2,
    msoAnimTypeScale = 3,
    msoAnimTypeRotation = 4,
    msoAnimTypeProperty = 5,
    msoAnimTypeCommand = 6,
    msoAnimTypeFilter = 7,
    msoAnimTypeSet = 8
};

enum __declspec(uuid("f9d204a8-2d7b-4f32-abe4-94650f0e2b72"))
MsoAnimAdditive
{
    msoAnimAdditiveAddBase = 1,
    msoAnimAdditiveAddSum = 2
};

enum __declspec(uuid("661ecfc7-4d4f-4bf9-b5f1-d4718ee3f9ca"))
MsoAnimAccumulate
{
    msoAnimAccumulateNone = 1,
    msoAnimAccumulateAlways = 2
};

struct __declspec(uuid("914934e5-5a91-11cf-8700-00aa0060263b"))
MotionEffect : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_ByX (
        /*[out,retval]*/ float * ByX ) = 0;
      virtual HRESULT __stdcall put_ByX (
        /*[in]*/ float ByX ) = 0;
      virtual HRESULT __stdcall get_ByY (
        /*[out,retval]*/ float * ByY ) = 0;
      virtual HRESULT __stdcall put_ByY (
        /*[in]*/ float ByY ) = 0;
      virtual HRESULT __stdcall get_FromX (
        /*[out,retval]*/ float * FromX ) = 0;
      virtual HRESULT __stdcall put_FromX (
        /*[in]*/ float FromX ) = 0;
      virtual HRESULT __stdcall get_FromY (
        /*[out,retval]*/ float * FromY ) = 0;
      virtual HRESULT __stdcall put_FromY (
        /*[in]*/ float FromY ) = 0;
      virtual HRESULT __stdcall get_ToX (
        /*[out,retval]*/ float * ToX ) = 0;
      virtual HRESULT __stdcall put_ToX (
        /*[in]*/ float ToX ) = 0;
      virtual HRESULT __stdcall get_ToY (
        /*[out,retval]*/ float * ToY ) = 0;
      virtual HRESULT __stdcall put_ToY (
        /*[in]*/ float ToY ) = 0;
      virtual HRESULT __stdcall get_Path (
        /*[out,retval]*/ BSTR * Path ) = 0;
      virtual HRESULT __stdcall put_Path (
        /*[in]*/ BSTR Path ) = 0;
};

struct __declspec(uuid("914934e6-5a91-11cf-8700-00aa0060263b"))
ColorEffect : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_By (
        /*[out,retval]*/ struct ColorFormat * * By ) = 0;
      virtual HRESULT __stdcall get_From (
        /*[out,retval]*/ struct ColorFormat * * From ) = 0;
      virtual HRESULT __stdcall get_To (
        /*[out,retval]*/ struct ColorFormat * * To ) = 0;
};

struct __declspec(uuid("914934e7-5a91-11cf-8700-00aa0060263b"))
ScaleEffect : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_ByX (
        /*[out,retval]*/ float * ByX ) = 0;
      virtual HRESULT __stdcall put_ByX (
        /*[in]*/ float ByX ) = 0;
      virtual HRESULT __stdcall get_ByY (
        /*[out,retval]*/ float * ByY ) = 0;
      virtual HRESULT __stdcall put_ByY (
        /*[in]*/ float ByY ) = 0;
      virtual HRESULT __stdcall get_FromX (
        /*[out,retval]*/ float * FromX ) = 0;
      virtual HRESULT __stdcall put_FromX (
        /*[in]*/ float FromX ) = 0;
      virtual HRESULT __stdcall get_FromY (
        /*[out,retval]*/ float * FromY ) = 0;
      virtual HRESULT __stdcall put_FromY (
        /*[in]*/ float FromY ) = 0;
      virtual HRESULT __stdcall get_ToX (
        /*[out,retval]*/ float * ToX ) = 0;
      virtual HRESULT __stdcall put_ToX (
        /*[in]*/ float ToX ) = 0;
      virtual HRESULT __stdcall get_ToY (
        /*[out,retval]*/ float * ToY ) = 0;
      virtual HRESULT __stdcall put_ToY (
        /*[in]*/ float ToY ) = 0;
};

struct __declspec(uuid("914934e8-5a91-11cf-8700-00aa0060263b"))
RotationEffect : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_By (
        /*[out,retval]*/ float * By ) = 0;
      virtual HRESULT __stdcall put_By (
        /*[in]*/ float By ) = 0;
      virtual HRESULT __stdcall get_From (
        /*[out,retval]*/ float * From ) = 0;
      virtual HRESULT __stdcall put_From (
        /*[in]*/ float From ) = 0;
      virtual HRESULT __stdcall get_To (
        /*[out,retval]*/ float * To ) = 0;
      virtual HRESULT __stdcall put_To (
        /*[in]*/ float To ) = 0;
};

enum __declspec(uuid("bd7baabc-c4f9-402e-a98c-b9840a10d355"))
MsoAnimProperty
{
    msoAnimNone = 0,
    msoAnimX = 1,
    msoAnimY = 2,
    msoAnimWidth = 3,
    msoAnimHeight = 4,
    msoAnimOpacity = 5,
    msoAnimRotation = 6,
    msoAnimColor = 7,
    msoAnimVisibility = 8,
    msoAnimTextFontBold = 100,
    msoAnimTextFontColor = 101,
    msoAnimTextFontEmboss = 102,
    msoAnimTextFontItalic = 103,
    msoAnimTextFontName = 104,
    msoAnimTextFontShadow = 105,
    msoAnimTextFontSize = 106,
    msoAnimTextFontSubscript = 107,
    msoAnimTextFontSuperscript = 108,
    msoAnimTextFontUnderline = 109,
    msoAnimTextFontStrikeThrough = 110,
    msoAnimTextBulletCharacter = 111,
    msoAnimTextBulletFontName = 112,
    msoAnimTextBulletNumber = 113,
    msoAnimTextBulletColor = 114,
    msoAnimTextBulletRelativeSize = 115,
    msoAnimTextBulletStyle = 116,
    msoAnimTextBulletType = 117,
    msoAnimShapePictureContrast = 1000,
    msoAnimShapePictureBrightness = 1001,
    msoAnimShapePictureGamma = 1002,
    msoAnimShapePictureGrayscale = 1003,
    msoAnimShapeFillOn = 1004,
    msoAnimShapeFillColor = 1005,
    msoAnimShapeFillOpacity = 1006,
    msoAnimShapeFillBackColor = 1007,
    msoAnimShapeLineOn = 1008,
    msoAnimShapeLineColor = 1009,
    msoAnimShapeShadowOn = 1010,
    msoAnimShapeShadowType = 1011,
    msoAnimShapeShadowColor = 1012,
    msoAnimShapeShadowOpacity = 1013,
    msoAnimShapeShadowOffsetX = 1014,
    msoAnimShapeShadowOffsetY = 1015
};

struct __declspec(uuid("914934eb-5a91-11cf-8700-00aa0060263b"))
AnimationPoint : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
      virtual HRESULT __stdcall get_Time (
        /*[out,retval]*/ float * Time ) = 0;
      virtual HRESULT __stdcall put_Time (
        /*[in]*/ float Time ) = 0;
      virtual HRESULT __stdcall get_Value (
        /*[out,retval]*/ VARIANT * Value ) = 0;
      virtual HRESULT __stdcall put_Value (
        /*[in]*/ VARIANT Value ) = 0;
      virtual HRESULT __stdcall get_Formula (
        /*[out,retval]*/ BSTR * Formula ) = 0;
      virtual HRESULT __stdcall put_Formula (
        /*[in]*/ BSTR Formula ) = 0;
};

struct __declspec(uuid("914934ea-5a91-11cf-8700-00aa0060263b"))
AnimationPoints : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct AnimationPoint * * Item ) = 0;
      virtual HRESULT __stdcall Add (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct AnimationPoint * * Add ) = 0;
      virtual HRESULT __stdcall get_Smooth (
        /*[out,retval]*/ enum Office2010::MsoTriState * Smooth ) = 0;
      virtual HRESULT __stdcall put_Smooth (
        /*[in]*/ enum Office2010::MsoTriState Smooth ) = 0;
};

struct __declspec(uuid("914934e9-5a91-11cf-8700-00aa0060263b"))
PropertyEffect : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Property (
        /*[out,retval]*/ enum MsoAnimProperty * Property ) = 0;
      virtual HRESULT __stdcall put_Property (
        /*[in]*/ enum MsoAnimProperty Property ) = 0;
      virtual HRESULT __stdcall get_Points (
        /*[out,retval]*/ struct AnimationPoints * * Points ) = 0;
      virtual HRESULT __stdcall get_From (
        /*[out,retval]*/ VARIANT * From ) = 0;
      virtual HRESULT __stdcall put_From (
        /*[in]*/ VARIANT From ) = 0;
      virtual HRESULT __stdcall get_To (
        /*[out,retval]*/ VARIANT * To ) = 0;
      virtual HRESULT __stdcall put_To (
        /*[in]*/ VARIANT To ) = 0;
};

enum __declspec(uuid("14876707-af12-41ee-983d-e7366840a4df"))
PpAlertLevel
{
    ppAlertsNone = 1,
    ppAlertsAll = 2
};

enum __declspec(uuid("ddfe9852-7847-4826-a6ab-0322e069c378"))
PpRevisionInfo
{
    ppRevisionInfoNone = 0,
    ppRevisionInfoBaseline = 1,
    ppRevisionInfoMerged = 2
};

struct __declspec(uuid("914934ed-5a91-11cf-8700-00aa0060263b"))
AutoCorrect : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_DisplayAutoCorrectOptions (
        /*[out,retval]*/ VARIANT_BOOL * DisplayAutoCorrectOptions ) = 0;
      virtual HRESULT __stdcall put_DisplayAutoCorrectOptions (
        /*[in]*/ VARIANT_BOOL DisplayAutoCorrectOptions ) = 0;
      virtual HRESULT __stdcall get_DisplayAutoLayoutOptions (
        /*[out,retval]*/ VARIANT_BOOL * DisplayAutoLayoutOptions ) = 0;
      virtual HRESULT __stdcall put_DisplayAutoLayoutOptions (
        /*[in]*/ VARIANT_BOOL DisplayAutoLayoutOptions ) = 0;
};

struct __declspec(uuid("914934ee-5a91-11cf-8700-00aa0060263b"))
Options : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_DisplayPasteOptions (
        /*[out,retval]*/ enum Office2010::MsoTriState * DisplayPasteOptions ) = 0;
      virtual HRESULT __stdcall put_DisplayPasteOptions (
        /*[in]*/ enum Office2010::MsoTriState DisplayPasteOptions ) = 0;
      virtual HRESULT __stdcall get_DoNotPromptForConvert (
        /*[out,retval]*/ enum Office2010::MsoTriState * DoNotPromptForConvert ) = 0;
      virtual HRESULT __stdcall put_DoNotPromptForConvert (
        /*[in]*/ enum Office2010::MsoTriState DoNotPromptForConvert ) = 0;
      virtual HRESULT __stdcall get_ShowCoauthoringMergeChanges (
        /*[out,retval]*/ VARIANT_BOOL * ShowCoauthoringMergeChanges ) = 0;
      virtual HRESULT __stdcall put_ShowCoauthoringMergeChanges (
        /*[in]*/ VARIANT_BOOL ShowCoauthoringMergeChanges ) = 0;
};

enum __declspec(uuid("25daee6c-e895-441f-bf3e-69fb4253dfe5"))
MsoAnimCommandType
{
    msoAnimCommandTypeEvent = 0,
    msoAnimCommandTypeCall = 1,
    msoAnimCommandTypeVerb = 2
};

struct __declspec(uuid("914934ef-5a91-11cf-8700-00aa0060263b"))
CommandEffect : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum MsoAnimCommandType * Type ) = 0;
      virtual HRESULT __stdcall put_Type (
        /*[in]*/ enum MsoAnimCommandType Type ) = 0;
      virtual HRESULT __stdcall get_Command (
        /*[out,retval]*/ BSTR * Command ) = 0;
      virtual HRESULT __stdcall put_Command (
        /*[in]*/ BSTR Command ) = 0;
      virtual HRESULT __stdcall get_bookmark (
        /*[out,retval]*/ BSTR * bookmark ) = 0;
      virtual HRESULT __stdcall put_bookmark (
        /*[in]*/ BSTR bookmark ) = 0;
};

enum __declspec(uuid("2f291805-eabf-4f68-801d-a2cc04340f2b"))
MsoAnimFilterEffectType
{
    msoAnimFilterEffectTypeNone = 0,
    msoAnimFilterEffectTypeBarn = 1,
    msoAnimFilterEffectTypeBlinds = 2,
    msoAnimFilterEffectTypeBox = 3,
    msoAnimFilterEffectTypeCheckerboard = 4,
    msoAnimFilterEffectTypeCircle = 5,
    msoAnimFilterEffectTypeDiamond = 6,
    msoAnimFilterEffectTypeDissolve = 7,
    msoAnimFilterEffectTypeFade = 8,
    msoAnimFilterEffectTypeImage = 9,
    msoAnimFilterEffectTypePixelate = 10,
    msoAnimFilterEffectTypePlus = 11,
    msoAnimFilterEffectTypeRandomBar = 12,
    msoAnimFilterEffectTypeSlide = 13,
    msoAnimFilterEffectTypeStretch = 14,
    msoAnimFilterEffectTypeStrips = 15,
    msoAnimFilterEffectTypeWedge = 16,
    msoAnimFilterEffectTypeWheel = 17,
    msoAnimFilterEffectTypeWipe = 18
};

enum __declspec(uuid("fcfb2414-4ebb-4875-b0a4-a697ca47af6a"))
MsoAnimFilterEffectSubtype
{
    msoAnimFilterEffectSubtypeNone = 0,
    msoAnimFilterEffectSubtypeInVertical = 1,
    msoAnimFilterEffectSubtypeOutVertical = 2,
    msoAnimFilterEffectSubtypeInHorizontal = 3,
    msoAnimFilterEffectSubtypeOutHorizontal = 4,
    msoAnimFilterEffectSubtypeHorizontal = 5,
    msoAnimFilterEffectSubtypeVertical = 6,
    msoAnimFilterEffectSubtypeIn = 7,
    msoAnimFilterEffectSubtypeOut = 8,
    msoAnimFilterEffectSubtypeAcross = 9,
    msoAnimFilterEffectSubtypeFromLeft = 10,
    msoAnimFilterEffectSubtypeFromRight = 11,
    msoAnimFilterEffectSubtypeFromTop = 12,
    msoAnimFilterEffectSubtypeFromBottom = 13,
    msoAnimFilterEffectSubtypeDownLeft = 14,
    msoAnimFilterEffectSubtypeUpLeft = 15,
    msoAnimFilterEffectSubtypeDownRight = 16,
    msoAnimFilterEffectSubtypeUpRight = 17,
    msoAnimFilterEffectSubtypeSpokes1 = 18,
    msoAnimFilterEffectSubtypeSpokes2 = 19,
    msoAnimFilterEffectSubtypeSpokes3 = 20,
    msoAnimFilterEffectSubtypeSpokes4 = 21,
    msoAnimFilterEffectSubtypeSpokes8 = 22,
    msoAnimFilterEffectSubtypeLeft = 23,
    msoAnimFilterEffectSubtypeRight = 24,
    msoAnimFilterEffectSubtypeDown = 25,
    msoAnimFilterEffectSubtypeUp = 26
};

struct __declspec(uuid("914934f0-5a91-11cf-8700-00aa0060263b"))
FilterEffect : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum MsoAnimFilterEffectType * Type ) = 0;
      virtual HRESULT __stdcall put_Type (
        /*[in]*/ enum MsoAnimFilterEffectType Type ) = 0;
      virtual HRESULT __stdcall get_Subtype (
        /*[out,retval]*/ enum MsoAnimFilterEffectSubtype * Subtype ) = 0;
      virtual HRESULT __stdcall put_Subtype (
        /*[in]*/ enum MsoAnimFilterEffectSubtype Subtype ) = 0;
      virtual HRESULT __stdcall get_Reveal (
        /*[out,retval]*/ enum Office2010::MsoTriState * Reveal ) = 0;
      virtual HRESULT __stdcall put_Reveal (
        /*[in]*/ enum Office2010::MsoTriState Reveal ) = 0;
};

struct __declspec(uuid("914934f1-5a91-11cf-8700-00aa0060263b"))
SetEffect : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Property (
        /*[out,retval]*/ enum MsoAnimProperty * Property ) = 0;
      virtual HRESULT __stdcall put_Property (
        /*[in]*/ enum MsoAnimProperty Property ) = 0;
      virtual HRESULT __stdcall get_To (
        /*[out,retval]*/ VARIANT * To ) = 0;
      virtual HRESULT __stdcall put_To (
        /*[in]*/ VARIANT To ) = 0;
};

enum __declspec(uuid("3b006e87-e68c-4933-9095-bbb3336fe234"))
PpRemoveDocInfoType
{
    ppRDIComments = 1,
    ppRDIRemovePersonalInformation = 4,
    ppRDIDocumentProperties = 8,
    ppRDIDocumentWorkspace = 10,
    ppRDIInkAnnotations = 11,
    ppRDIPublishPath = 13,
    ppRDIDocumentServerProperties = 14,
    ppRDIDocumentManagementPolicy = 15,
    ppRDIContentType = 16,
    ppRDISlideUpdateInformation = 17,
    ppRDIAll = 99
};

enum __declspec(uuid("f4694af5-8686-44e1-978f-eeba63557b90"))
PpCheckInVersionType
{
    ppCheckInMinorVersion = 0,
    ppCheckInMajorVersion = 1,
    ppCheckInOverwriteVersion = 2
};

enum __declspec(uuid("abf6bcae-eb69-4044-bd66-87db3a1e0211"))
MsoClickState
{
    msoClickStateAfterAllAnimations = -2,
    msoClickStateBeforeAutomaticAnimations = -1
};

enum __declspec(uuid("cd470fbb-4284-4670-87e5-f315ef63d556"))
PpFixedFormatType
{
    ppFixedFormatTypeXPS = 1,
    ppFixedFormatTypePDF = 2
};

enum __declspec(uuid("2cf9706b-e805-4ef6-ba87-7b7ebbd43d42"))
PpFixedFormatIntent
{
    ppFixedFormatIntentScreen = 1,
    ppFixedFormatIntentPrint = 2
};

struct __declspec(uuid("914934f5-5a91-11cf-8700-00aa0060263b"))
TableStyle : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall get_Id (
        /*[out,retval]*/ BSTR * Id ) = 0;
};

struct __declspec(uuid("914934f6-5a91-11cf-8700-00aa0060263b"))
CustomerData : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ BSTR Id,
        /*[out,retval]*/ struct Office2010::_CustomXMLPart * * Item ) = 0;
      virtual HRESULT __stdcall Add (
        /*[out,retval]*/ struct Office2010::_CustomXMLPart * * Add ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[in]*/ BSTR Id ) = 0;
};

struct __declspec(uuid("914934f7-5a91-11cf-8700-00aa0060263b"))
Research : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Query (
        /*[in]*/ BSTR ServiceID,
        /*[in]*/ VARIANT * QueryString,
        /*[in]*/ VARIANT * QueryLanguage,
        /*[in]*/ VARIANT_BOOL UseSelection,
        /*[in]*/ VARIANT_BOOL LaunchQuery ) = 0;
      virtual HRESULT __stdcall SetLanguagePair (
        /*[in]*/ VARIANT * Language1,
        /*[in]*/ VARIANT * Language2 ) = 0;
      virtual HRESULT __stdcall IsResearchService (
        /*[in]*/ BSTR ServiceID,
        /*[out,retval]*/ VARIANT_BOOL * IsResearchService ) = 0;
};

struct __declspec(uuid("914934f8-5a91-11cf-8700-00aa0060263b"))
TableBackground : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Fill (
        /*[out,retval]*/ struct FillFormat * * Fill ) = 0;
      virtual HRESULT __stdcall get_Picture (
        /*[out,retval]*/ struct PictureFormat * * Picture ) = 0;
      virtual HRESULT __stdcall get_Reflection (
        /*[out,retval]*/ struct Office2010::ReflectionFormat * * Reflection ) = 0;
      virtual HRESULT __stdcall get_Shadow (
        /*[out,retval]*/ struct ShadowFormat * * Shadow ) = 0;
};

struct __declspec(uuid("914934f9-5a91-11cf-8700-00aa0060263b"))
TextFrame2 : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_MarginBottom (
        /*[out,retval]*/ float * MarginBottom ) = 0;
      virtual HRESULT __stdcall put_MarginBottom (
        /*[in]*/ float MarginBottom ) = 0;
      virtual HRESULT __stdcall get_MarginLeft (
        /*[out,retval]*/ float * MarginLeft ) = 0;
      virtual HRESULT __stdcall put_MarginLeft (
        /*[in]*/ float MarginLeft ) = 0;
      virtual HRESULT __stdcall get_MarginRight (
        /*[out,retval]*/ float * MarginRight ) = 0;
      virtual HRESULT __stdcall put_MarginRight (
        /*[in]*/ float MarginRight ) = 0;
      virtual HRESULT __stdcall get_MarginTop (
        /*[out,retval]*/ float * MarginTop ) = 0;
      virtual HRESULT __stdcall put_MarginTop (
        /*[in]*/ float MarginTop ) = 0;
      virtual HRESULT __stdcall get_Orientation (
        /*[out,retval]*/ enum Office2010::MsoTextOrientation * Orientation ) = 0;
      virtual HRESULT __stdcall put_Orientation (
        /*[in]*/ enum Office2010::MsoTextOrientation Orientation ) = 0;
      virtual HRESULT __stdcall get_HorizontalAnchor (
        /*[out,retval]*/ enum Office2010::MsoHorizontalAnchor * HorizontalAnchor ) = 0;
      virtual HRESULT __stdcall put_HorizontalAnchor (
        /*[in]*/ enum Office2010::MsoHorizontalAnchor HorizontalAnchor ) = 0;
      virtual HRESULT __stdcall get_VerticalAnchor (
        /*[out,retval]*/ enum Office2010::MsoVerticalAnchor * VerticalAnchor ) = 0;
      virtual HRESULT __stdcall put_VerticalAnchor (
        /*[in]*/ enum Office2010::MsoVerticalAnchor VerticalAnchor ) = 0;
      virtual HRESULT __stdcall get_PathFormat (
        /*[out,retval]*/ enum Office2010::MsoPathFormat * PathFormat ) = 0;
      virtual HRESULT __stdcall put_PathFormat (
        /*[in]*/ enum Office2010::MsoPathFormat PathFormat ) = 0;
      virtual HRESULT __stdcall get_WarpFormat (
        /*[out,retval]*/ enum Office2010::MsoWarpFormat * WarpFormat ) = 0;
      virtual HRESULT __stdcall put_WarpFormat (
        /*[in]*/ enum Office2010::MsoWarpFormat WarpFormat ) = 0;
      virtual HRESULT __stdcall get_WordArtFormat (
        /*[out,retval]*/ enum Office2010::MsoPresetTextEffect * WordArtFormat ) = 0;
      virtual HRESULT __stdcall put_WordArtFormat (
        /*[in]*/ enum Office2010::MsoPresetTextEffect WordArtFormat ) = 0;
      virtual HRESULT __stdcall get_WordWrap (
        /*[out,retval]*/ enum Office2010::MsoTriState * WordWrap ) = 0;
      virtual HRESULT __stdcall put_WordWrap (
        /*[in]*/ enum Office2010::MsoTriState WordWrap ) = 0;
      virtual HRESULT __stdcall get_AutoSize (
        /*[out,retval]*/ enum Office2010::MsoAutoSize * AutoSize ) = 0;
      virtual HRESULT __stdcall put_AutoSize (
        /*[in]*/ enum Office2010::MsoAutoSize AutoSize ) = 0;
      virtual HRESULT __stdcall get_ThreeD (
        /*[out,retval]*/ struct ThreeDFormat * * ThreeD ) = 0;
      virtual HRESULT __stdcall get_HasText (
        /*[out,retval]*/ enum Office2010::MsoTriState * pHasText ) = 0;
      virtual HRESULT __stdcall get_TextRange (
        /*[out,retval]*/ struct Office2010::TextRange2 * * Range ) = 0;
      virtual HRESULT __stdcall get_Column (
        /*[out,retval]*/ struct Office2010::TextColumn2 * * Column ) = 0;
      virtual HRESULT __stdcall get_Ruler (
        /*[out,retval]*/ struct Office2010::Ruler2 * * Ruler ) = 0;
      virtual HRESULT __stdcall DeleteText ( ) = 0;
      virtual HRESULT __stdcall get_NoTextRotation (
        /*[out,retval]*/ enum Office2010::MsoTriState * NoTextRotation ) = 0;
      virtual HRESULT __stdcall put_NoTextRotation (
        /*[in]*/ enum Office2010::MsoTriState NoTextRotation ) = 0;
};

enum __declspec(uuid("628c4ecd-2840-41ab-ba67-fe836c277e76"))
XlBackground
{
    xlBackgroundAutomatic = -4105,
    xlBackgroundOpaque = 3,
    xlBackgroundTransparent = 2
};

enum __declspec(uuid("b1ac486b-feb5-4f91-ab5a-674bd3c93ff7"))
XlChartGallery
{
    xlBuiltIn = 21,
    xlUserDefined = 22,
    xlAnyGallery = 23
};

enum __declspec(uuid("361d6719-c6c7-438c-849f-09f55b485c30"))
XlChartPicturePlacement
{
    xlSides = 1,
    xlEnd = 2,
    xlEndSides = 3,
    xlFront = 4,
    xlFrontSides = 5,
    xlFrontEnd = 6,
    xlAllFaces = 7
};

enum __declspec(uuid("6d851b33-e263-4602-b4bd-96ed7cb4f542"))
XlDataLabelSeparator
{
    xlDataLabelSeparatorDefault = 1
};

enum __declspec(uuid("f800b4fc-157e-49f8-80a8-07a524f87c63"))
XlPattern
{
    xlPatternAutomatic = -4105,
    xlPatternChecker = 9,
    xlPatternCrissCross = 16,
    xlPatternDown = -4121,
    xlPatternGray16 = 17,
    xlPatternGray25 = -4124,
    xlPatternGray50 = -4125,
    xlPatternGray75 = -4126,
    xlPatternGray8 = 18,
    xlPatternGrid = 15,
    xlPatternHorizontal = -4128,
    xlPatternLightDown = 13,
    xlPatternLightHorizontal = 11,
    xlPatternLightUp = 14,
    xlPatternLightVertical = 12,
    xlPatternNone = -4142,
    xlPatternSemiGray75 = 10,
    xlPatternSolid = 1,
    xlPatternUp = -4162,
    xlPatternVertical = -4166,
    xlPatternLinearGradient = 4000,
    xlPatternRectangularGradient = 4001
};

enum __declspec(uuid("a7e68d3b-0a7d-4f2e-a398-7fa9f2db0d8a"))
XlPictureAppearance
{
    xlPrinter = 2,
    xlScreen = 1
};

enum __declspec(uuid("0acdcd15-22e1-467c-a50b-9940132630a2"))
XlCopyPictureFormat
{
    xlBitmap = 2,
    xlPicture = -4147
};

enum __declspec(uuid("1e1c4c4b-742d-40ca-8dd8-6e9b772d117d"))
XlRgbColor
{
    rgbAliceBlue = 16775408,
    rgbAntiqueWhite = 14150650,
    rgbAqua = 16776960,
    rgbAquamarine = 13959039,
    rgbAzure = 16777200,
    rgbBeige = 14480885,
    rgbBisque = 12903679,
    rgbBlack = 0,
    rgbBlanchedAlmond = 13495295,
    rgbBlue = 16711680,
    rgbBlueViolet = 14822282,
    rgbBrown = 2763429,
    rgbBurlyWood = 8894686,
    rgbCadetBlue = 10526303,
    rgbChartreuse = 65407,
    rgbCoral = 5275647,
    rgbCornflowerBlue = 15570276,
    rgbCornsilk = 14481663,
    rgbCrimson = 3937500,
    rgbDarkBlue = 9109504,
    rgbDarkCyan = 9145088,
    rgbDarkGoldenrod = 755384,
    rgbDarkGreen = 25600,
    rgbDarkGray = 11119017,
    rgbDarkGrey = 11119017,
    rgbDarkKhaki = 7059389,
    rgbDarkMagenta = 9109643,
    rgbDarkOliveGreen = 3107669,
    rgbDarkOrange = 36095,
    rgbDarkOrchid = 13382297,
    rgbDarkRed = 139,
    rgbDarkSalmon = 8034025,
    rgbDarkSeaGreen = 9419919,
    rgbDarkSlateBlue = 9125192,
    rgbDarkSlateGray = 5197615,
    rgbDarkSlateGrey = 5197615,
    rgbDarkTurquoise = 13749760,
    rgbDarkViolet = 13828244,
    rgbDeepPink = 9639167,
    rgbDeepSkyBlue = 16760576,
    rgbDimGray = 6908265,
    rgbDimGrey = 6908265,
    rgbDodgerBlue = 16748574,
    rgbFireBrick = 2237106,
    rgbFloralWhite = 15792895,
    rgbForestGreen = 2263842,
    rgbFuchsia = 16711935,
    rgbGainsboro = 14474460,
    rgbGhostWhite = 16775416,
    rgbGold = 55295,
    rgbGoldenrod = 2139610,
    rgbGray = 8421504,
    rgbGreen = 32768,
    rgbGrey = 8421504,
    rgbGreenYellow = 3145645,
    rgbHoneydew = 15794160,
    rgbHotPink = 11823615,
    rgbIndianRed = 6053069,
    rgbIndigo = 8519755,
    rgbIvory = 15794175,
    rgbKhaki = 9234160,
    rgbLavender = 16443110,
    rgbLavenderBlush = 16118015,
    rgbLawnGreen = 64636,
    rgbLemonChiffon = 13499135,
    rgbLightBlue = 15128749,
    rgbLightCoral = 8421616,
    rgbLightCyan = 9145088,
    rgbLightGoldenrodYellow = 13826810,
    rgbLightGray = 13882323,
    rgbLightGreen = 9498256,
    rgbLightGrey = 13882323,
    rgbLightPink = 12695295,
    rgbLightSalmon = 8036607,
    rgbLightSeaGreen = 11186720,
    rgbLightSkyBlue = 16436871,
    rgbLightSlateGray = 10061943,
    rgbLightSlateGrey = 10061943,
    rgbLightSteelBlue = 14599344,
    rgbLightYellow = 14745599,
    rgbLime = 65280,
    rgbLimeGreen = 3329330,
    rgbLinen = 15134970,
    rgbMaroon = 128,
    rgbMediumAquamarine = 11206502,
    rgbMediumBlue = 13434880,
    rgbMediumOrchid = 13850042,
    rgbMediumPurple = 14381203,
    rgbMediumSeaGreen = 7451452,
    rgbMediumSlateBlue = 15624315,
    rgbMediumSpringGreen = 10156544,
    rgbMediumTurquoise = 13422920,
    rgbMediumVioletRed = 8721863,
    rgbMidnightBlue = 7346457,
    rgbMintCream = 16449525,
    rgbMistyRose = 14804223,
    rgbMoccasin = 11920639,
    rgbNavajoWhite = 11394815,
    rgbNavy = 8388608,
    rgbNavyBlue = 8388608,
    rgbOldLace = 15136253,
    rgbOlive = 32896,
    rgbOliveDrab = 2330219,
    rgbOrange = 42495,
    rgbOrangeRed = 17919,
    rgbOrchid = 14053594,
    rgbPaleGoldenrod = 7071982,
    rgbPaleGreen = 10025880,
    rgbPaleTurquoise = 15658671,
    rgbPaleVioletRed = 9662683,
    rgbPapayaWhip = 14020607,
    rgbPeachPuff = 12180223,
    rgbPeru = 4163021,
    rgbPink = 13353215,
    rgbPlum = 14524637,
    rgbPowderBlue = 15130800,
    rgbPurple = 8388736,
    rgbRed = 255,
    rgbRosyBrown = 9408444,
    rgbRoyalBlue = 14772545,
    rgbSalmon = 7504122,
    rgbSandyBrown = 6333684,
    rgbSeaGreen = 5737262,
    rgbSeashell = 15660543,
    rgbSienna = 2970272,
    rgbSilver = 12632256,
    rgbSkyBlue = 15453831,
    rgbSlateBlue = 13458026,
    rgbSlateGray = 9470064,
    rgbSlateGrey = 9470064,
    rgbSnow = 16448255,
    rgbSpringGreen = 8388352,
    rgbSteelBlue = 11829830,
    rgbTan = 9221330,
    rgbTeal = 8421376,
    rgbThistle = 14204888,
    rgbTomato = 4678655,
    rgbTurquoise = 13688896,
    rgbYellow = 65535,
    rgbYellowGreen = 3329434,
    rgbViolet = 15631086,
    rgbWheat = 11788021,
    rgbWhite = 16777215,
    rgbWhiteSmoke = 16119285
};

enum __declspec(uuid("850b18fb-afb2-489c-a498-df16aff614f3"))
XlLineStyle
{
    xlContinuous = 1,
    xlDash = -4115,
    xlDashDot = 4,
    xlDashDotDot = 5,
    xlDot = -4118,
    xlDouble = -4119,
    xlSlantDashDot = 13,
    xlLineStyleNone = -4142
};

enum __declspec(uuid("b95c5967-64bb-4d6f-9ee8-9916b8637a41"))
XlAxisCrosses
{
    xlAxisCrossesAutomatic = -4105,
    xlAxisCrossesCustom = -4114,
    xlAxisCrossesMaximum = 2,
    xlAxisCrossesMinimum = 4
};

enum __declspec(uuid("9f3a2942-5ba4-4578-b6e3-4819c4cad7b0"))
XlAxisGroup
{
    xlPrimary = 1,
    xlSecondary = 2
};

enum __declspec(uuid("dd79733b-5e46-49c9-8400-6bcf316ec79e"))
XlAxisType
{
    xlCategory = 1,
    xlSeriesAxis = 3,
    xlValue = 2
};

enum __declspec(uuid("d2431a3d-f112-4a83-97b2-bc3ce3c9b73b"))
XlBarShape
{
    xlBox = 0,
    xlPyramidToPoint = 1,
    xlPyramidToMax = 2,
    xlCylinder = 3,
    xlConeToPoint = 4,
    xlConeToMax = 5
};

enum __declspec(uuid("3918e32a-fd7f-461c-b2e0-f5605207c30b"))
XlBorderWeight
{
    xlHairline = 1,
    xlMedium = -4138,
    xlThick = 4,
    xlThin = 2
};

enum __declspec(uuid("406223e5-9460-41d9-babf-b6e6d3c62f2d"))
XlCategoryType
{
    xlCategoryScale = 2,
    xlTimeScale = 3,
    xlAutomaticScale = -4105
};

enum __declspec(uuid("a66eb34b-bcc6-40e7-9722-398cf51f2a17"))
XlChartElementPosition
{
    xlChartElementPositionAutomatic = -4105,
    xlChartElementPositionCustom = -4114
};

enum __declspec(uuid("fb55f3eb-2f7c-4410-982e-ed9bc7812e32"))
XlChartItem
{
    xlDataLabel = 0,
    xlChartArea = 2,
    xlSeries = 3,
    xlChartTitle = 4,
    xlWalls = 5,
    xlCorners = 6,
    xlDataTable = 7,
    xlTrendline = 8,
    xlErrorBars = 9,
    xlXErrorBars = 10,
    xlYErrorBars = 11,
    xlLegendEntry = 12,
    xlLegendKey = 13,
    xlShape = 14,
    xlMajorGridlines = 15,
    xlMinorGridlines = 16,
    xlAxisTitle = 17,
    xlUpBars = 18,
    xlPlotArea = 19,
    xlDownBars = 20,
    xlAxis = 21,
    xlSeriesLines = 22,
    xlFloor = 23,
    xlLegend = 24,
    xlHiLoLines = 25,
    xlDropLines = 26,
    xlRadarAxisLabels = 27,
    xlNothing = 28,
    xlLeaderLines = 29,
    xlDisplayUnitLabel = 30,
    xlPivotChartFieldButton = 31,
    xlPivotChartDropZone = 32
};

enum __declspec(uuid("bcc70dfc-c553-4fa1-99cc-c599a711add0"))
XlOrientation
{
    xlDownward = -4170,
    xlHorizontal = -4128,
    xlUpward = -4171,
    xlVertical = -4166
};

enum __declspec(uuid("de37e118-9080-45e2-8f09-67161638990f"))
XlChartPictureType
{
    xlStackScale = 3,
    xlStack = 2,
    xlStretch = 1
};

enum __declspec(uuid("878f9ad5-d422-4ca3-80a0-ae420918f413"))
XlChartSplitType
{
    xlSplitByPosition = 1,
    xlSplitByPercentValue = 3,
    xlSplitByCustomSplit = 4,
    xlSplitByValue = 2
};

enum __declspec(uuid("9dcda232-6504-4f31-a174-ceee2efe5f27"))
XlColorIndex
{
    xlColorIndexAutomatic = -4105,
    xlColorIndexNone = -4142
};

enum __declspec(uuid("183eb140-c143-4a88-a7e0-15fc6e00c217"))
XlConstants
{
    xlAutomatic = -4105,
    xlCombination = -4111,
    xlCustom = -4114,
    xlBar = 2,
    xlColumn = 3,
    xl3DBar = -4099,
    xl3DSurface = -4103,
    xlDefaultAutoFormat = -1,
    xlNone = -4142,
    xlAbove = 0,
    xlBelow = 1,
    xlBoth = 1,
    xlBottom = -4107,
    xlCenter = -4108,
    xlChecker = 9,
    xlCircle = 8,
    xlCorner = 2,
    xlCrissCross = 16,
    xlCross = 4,
    xlDiamond = 2,
    xlDistributed = -4117,
    xlFill = 5,
    xlFixedValue = 1,
    xlGeneral = 1,
    xlGray16 = 17,
    xlGray25 = -4124,
    xlGray50 = -4125,
    xlGray75 = -4126,
    xlGray8 = 18,
    xlGrid = 15,
    xlHigh = -4127,
    xlInside = 2,
    xlJustify = -4130,
    xlLeft = -4131,
    xlLightDown = 13,
    xlLightHorizontal = 11,
    xlLightUp = 14,
    xlLightVertical = 12,
    xlLow = -4134,
    xlMaximum = 2,
    xlMinimum = 4,
    xlMinusValues = 3,
    xlNextToAxis = 4,
    xlOpaque = 3,
    xlOutside = 3,
    xlPercent = 2,
    xlPlus = 9,
    xlPlusValues = 2,
    xlRight = -4152,
    xlScale = 3,
    xlSemiGray75 = 10,
    xlShowLabel = 4,
    xlShowLabelAndPercent = 5,
    xlShowPercent = 3,
    xlShowValue = 2,
    xlSingle = 2,
    xlSolid = 1,
    xlSquare = 1,
    xlStar = 5,
    xlStError = 4,
    xlTop = -4160,
    xlTransparent = 2,
    xlTriangle = 3
};

enum __declspec(uuid("f11b2572-8ff8-44ae-9876-aef9a8b749a8"))
XlDataLabelPosition
{
    xlLabelPositionCenter = -4108,
    xlLabelPositionAbove = 0,
    xlLabelPositionBelow = 1,
    xlLabelPositionLeft = -4131,
    xlLabelPositionRight = -4152,
    xlLabelPositionOutsideEnd = 2,
    xlLabelPositionInsideEnd = 3,
    xlLabelPositionInsideBase = 4,
    xlLabelPositionBestFit = 5,
    xlLabelPositionMixed = 6,
    xlLabelPositionCustom = 7
};

enum __declspec(uuid("840d3be9-75b9-44f8-a4a9-8c23f48dffe0"))
XlDataLabelsType
{
    xlDataLabelsShowNone = -4142,
    xlDataLabelsShowValue = 2,
    xlDataLabelsShowPercent = 3,
    xlDataLabelsShowLabel = 4,
    xlDataLabelsShowLabelAndPercent = 5,
    xlDataLabelsShowBubbleSizes = 6
};

enum __declspec(uuid("04b01227-b5bd-448c-89ab-d990e9e346f4"))
XlDisplayBlanksAs
{
    xlInterpolated = 3,
    xlNotPlotted = 1,
    xlZero = 2
};

enum __declspec(uuid("87a9a6ee-10d5-41c4-bd53-fdca6ec7e139"))
XlDisplayUnit
{
    xlHundreds = -2,
    xlThousands = -3,
    xlTenThousands = -4,
    xlHundredThousands = -5,
    xlMillions = -6,
    xlTenMillions = -7,
    xlHundredMillions = -8,
    xlThousandMillions = -9,
    xlMillionMillions = -10
};

enum __declspec(uuid("7bd721fc-e709-48b5-9358-18408f131030"))
XlEndStyleCap
{
    xlCap = 1,
    xlNoCap = 2
};

enum __declspec(uuid("3ad2fbbd-b1ae-4bed-925d-ce60450115cb"))
XlErrorBarDirection
{
    xlChartX = -4168,
    xlChartY = 1
};

enum __declspec(uuid("00c9e500-2e7d-4ecf-b7bc-d0b86cb8f3a6"))
XlErrorBarInclude
{
    xlErrorBarIncludeBoth = 1,
    xlErrorBarIncludeMinusValues = 3,
    xlErrorBarIncludeNone = -4142,
    xlErrorBarIncludePlusValues = 2
};

enum __declspec(uuid("00983aab-ca07-437d-9415-154dad6918f0"))
XlErrorBarType
{
    xlErrorBarTypeCustom = -4114,
    xlErrorBarTypeFixedValue = 1,
    xlErrorBarTypePercent = 2,
    xlErrorBarTypeStDev = -4155,
    xlErrorBarTypeStError = 4
};

enum __declspec(uuid("9d60ef74-103f-4acb-8042-20c4d100f034"))
XlHAlign
{
    xlHAlignCenter = -4108,
    xlHAlignCenterAcrossSelection = 7,
    xlHAlignDistributed = -4117,
    xlHAlignFill = 5,
    xlHAlignGeneral = 1,
    xlHAlignJustify = -4130,
    xlHAlignLeft = -4131,
    xlHAlignRight = -4152
};

enum __declspec(uuid("db4b1313-f371-453b-9eca-9c9495fc23ad"))
XlLegendPosition
{
    xlLegendPositionBottom = -4107,
    xlLegendPositionCorner = 2,
    xlLegendPositionLeft = -4131,
    xlLegendPositionRight = -4152,
    xlLegendPositionTop = -4160,
    xlLegendPositionCustom = -4161
};

enum __declspec(uuid("12b55737-a883-42ff-b5f7-d7360e7b97b4"))
XlMarkerStyle
{
    xlMarkerStyleAutomatic = -4105,
    xlMarkerStyleCircle = 8,
    xlMarkerStyleDash = -4115,
    xlMarkerStyleDiamond = 2,
    xlMarkerStyleDot = -4118,
    xlMarkerStyleNone = -4142,
    xlMarkerStylePicture = -4147,
    xlMarkerStylePlus = 9,
    xlMarkerStyleSquare = 1,
    xlMarkerStyleStar = 5,
    xlMarkerStyleTriangle = 3,
    xlMarkerStyleX = -4168
};

enum __declspec(uuid("7fac3690-46d1-49cd-8793-5690439ddc8b"))
XlPivotFieldOrientation
{
    xlColumnField = 2,
    xlDataField = 4,
    xlHidden = 0,
    xlPageField = 3,
    xlRowField = 1
};

enum __declspec(uuid("34a545f2-2447-4991-9cd2-b7f0d5707591"))
XlReadingOrder
{
    xlContext = -5002,
    xlLTR = -5003,
    xlRTL = -5004
};

enum __declspec(uuid("08474527-2498-4302-9408-aaa07de998d7"))
XlRowCol
{
    xlColumns = 2,
    xlRows = 1
};

enum __declspec(uuid("ca98c99c-d154-4b5e-a142-ed2e49fe6ea1"))
XlScaleType
{
    xlScaleLinear = -4132,
    xlScaleLogarithmic = -4133
};

enum __declspec(uuid("b5c2a6b9-6991-478a-86c0-c4a227f44940"))
XlSizeRepresents
{
    xlSizeIsWidth = 2,
    xlSizeIsArea = 1
};

enum __declspec(uuid("dc28ed70-abb0-41a1-b45e-73d98203b3b5"))
XlTickLabelOrientation
{
    xlTickLabelOrientationAutomatic = -4105,
    xlTickLabelOrientationDownward = -4170,
    xlTickLabelOrientationHorizontal = -4128,
    xlTickLabelOrientationUpward = -4171,
    xlTickLabelOrientationVertical = -4166
};

enum __declspec(uuid("fbb0d012-58f0-4251-aba7-0c0a5e416514"))
XlTickLabelPosition
{
    xlTickLabelPositionHigh = -4127,
    xlTickLabelPositionLow = -4134,
    xlTickLabelPositionNextToAxis = 4,
    xlTickLabelPositionNone = -4142
};

enum __declspec(uuid("6d1edf65-fb90-465c-8777-015d7028e975"))
XlTickMark
{
    xlTickMarkCross = 4,
    xlTickMarkInside = 2,
    xlTickMarkNone = -4142,
    xlTickMarkOutside = 3
};

enum __declspec(uuid("b5e8c11d-ec7b-491b-96e8-af57a773eed2"))
XlTimeUnit
{
    xlDays = 0,
    xlMonths = 1,
    xlYears = 2
};

enum __declspec(uuid("f2bfa1d6-b1e6-4375-96f0-11540cd587c2"))
XlTrendlineType
{
    xlExponential = 5,
    xlLinear = -4132,
    xlLogarithmic = -4133,
    xlMovingAvg = 6,
    xlPolynomial = 3,
    xlPower = 4
};

enum __declspec(uuid("ddafa8b9-3bbb-4679-8d6c-8701b9f234e2"))
XlUnderlineStyle
{
    xlUnderlineStyleDouble = -4119,
    xlUnderlineStyleDoubleAccounting = 5,
    xlUnderlineStyleNone = -4142,
    xlUnderlineStyleSingle = 2,
    xlUnderlineStyleSingleAccounting = 4
};

enum __declspec(uuid("e0662922-d918-4344-8a6a-d95499304ce2"))
XlVAlign
{
    xlVAlignBottom = -4107,
    xlVAlignCenter = -4108,
    xlVAlignDistributed = -4117,
    xlVAlignJustify = -4130,
    xlVAlignTop = -4160
};

struct __declspec(uuid("92d41a56-f07e-4ca4-af6f-bef486aa4e6f"))
ChartBorder : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall put_Color (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Color (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_ColorIndex (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_ColorIndex (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_LineStyle (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_LineStyle (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Weight (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Weight (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a59-f07e-4ca4-af6f-bef486aa4e6f"))
ChartColorFormat : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_SchemeColor (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_SchemeColor (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get__Default (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_RGB (
        /*[out,retval]*/ long * RHS ) = 0;
};

struct __declspec(uuid("92d41a5a-f07e-4ca4-af6f-bef486aa4e6f"))
ChartData : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Workbook (
        /*[out,retval]*/ IDispatch * * ppdispWorkbook ) = 0;
      virtual HRESULT __stdcall Activate ( ) = 0;
      virtual HRESULT __stdcall get_IsLinked (
        /*[out,retval]*/ VARIANT_BOOL * pfIsLinked ) = 0;
      virtual HRESULT __stdcall BreakLink ( ) = 0;
};

struct __declspec(uuid("92d41a5b-f07e-4ca4-af6f-bef486aa4e6f"))
ChartFillFormat : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall Solid ( ) = 0;
      virtual HRESULT __stdcall UserTextured (
        /*[in]*/ BSTR TextureFile ) = 0;
      virtual HRESULT __stdcall get_BackColor (
        /*[out,retval]*/ struct ChartColorFormat * * RHS ) = 0;
      virtual HRESULT __stdcall get_ForeColor (
        /*[out,retval]*/ struct ChartColorFormat * * RHS ) = 0;
      virtual HRESULT __stdcall get_GradientDegree (
        /*[out,retval]*/ float * RHS ) = 0;
      virtual HRESULT __stdcall get_TextureName (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall OneColorGradient (
        /*[in]*/ enum Office2010::MsoGradientStyle Style,
        /*[in]*/ long Variant,
        /*[in]*/ float Degree ) = 0;
      virtual HRESULT __stdcall get_GradientColorType (
        /*[out,retval]*/ enum Office2010::MsoGradientColorType * RHS ) = 0;
      virtual HRESULT __stdcall get_GradientStyle (
        /*[out,retval]*/ enum Office2010::MsoGradientStyle * RHS ) = 0;
      virtual HRESULT __stdcall get_GradientVariant (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_Pattern (
        /*[out,retval]*/ enum Office2010::MsoPatternType * RHS ) = 0;
      virtual HRESULT __stdcall Patterned (
        /*[in]*/ enum Office2010::MsoPatternType Pattern ) = 0;
      virtual HRESULT __stdcall PresetGradient (
        /*[in]*/ enum Office2010::MsoGradientStyle Style,
        /*[in]*/ long Variant,
        /*[in]*/ enum Office2010::MsoPresetGradientType PresetGradientType ) = 0;
      virtual HRESULT __stdcall get_PresetGradientType (
        /*[out,retval]*/ enum Office2010::MsoPresetGradientType * RHS ) = 0;
      virtual HRESULT __stdcall get_PresetTexture (
        /*[out,retval]*/ enum Office2010::MsoPresetTexture * RHS ) = 0;
      virtual HRESULT __stdcall PresetTextured (
        /*[in]*/ enum Office2010::MsoPresetTexture PresetTexture ) = 0;
      virtual HRESULT __stdcall get_TextureType (
        /*[out,retval]*/ enum Office2010::MsoTextureType * RHS ) = 0;
      virtual HRESULT __stdcall TwoColorGradient (
        /*[in]*/ enum Office2010::MsoGradientStyle Style,
        /*[in]*/ long Variant ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum Office2010::MsoFillType * RHS ) = 0;
      virtual HRESULT __stdcall UserPicture (
        /*[in]*/ VARIANT PictureFile = vtMissing,
        /*[in]*/ VARIANT PictureFormat = vtMissing,
        /*[in]*/ VARIANT PictureStackUnit = vtMissing,
        /*[in]*/ VARIANT PicturePlacement = vtMissing ) = 0;
      virtual HRESULT __stdcall get_Visible (
        /*[out,retval]*/ enum Office2010::MsoTriState * RHS ) = 0;
      virtual HRESULT __stdcall put_Visible (
        /*[in]*/ enum Office2010::MsoTriState RHS ) = 0;
};

struct __declspec(uuid("92d41a5c-f07e-4ca4-af6f-bef486aa4e6f"))
ChartFormat : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Fill (
        /*[out,retval]*/ struct FillFormat * * ppFill ) = 0;
      virtual HRESULT __stdcall get_Glow (
        /*[out,retval]*/ struct Office2010::GlowFormat * * ppGlow ) = 0;
      virtual HRESULT __stdcall get_Line (
        /*[out,retval]*/ struct LineFormat * * ppLine ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * ppparent ) = 0;
      virtual HRESULT __stdcall get_PictureFormat (
        /*[out,retval]*/ struct PictureFormat * * ppPictureFormat ) = 0;
      virtual HRESULT __stdcall get_Shadow (
        /*[out,retval]*/ struct ShadowFormat * * ppShadow ) = 0;
      virtual HRESULT __stdcall get_SoftEdge (
        /*[out,retval]*/ struct Office2010::SoftEdgeFormat * * ppSoftEdge ) = 0;
      virtual HRESULT __stdcall get_TextFrame2 (
        /*[out,retval]*/ struct TextFrame2 * * ppTextFrame ) = 0;
      virtual HRESULT __stdcall get_ThreeD (
        /*[out,retval]*/ struct ThreeDFormat * * ppThreeD ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a60-f07e-4ca4-af6f-bef486aa4e6f"))
Corners : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a66-f07e-4ca4-af6f-bef486aa4e6f"))
DropLines : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * RHS ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
};

struct __declspec(uuid("92d41a67-f07e-4ca4-af6f-bef486aa4e6f"))
ErrorBars : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * RHS ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall ClearFormats (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_EndStyle (
        /*[out,retval]*/ enum XlEndStyleCap * RHS ) = 0;
      virtual HRESULT __stdcall put_EndStyle (
        /*[in]*/ enum XlEndStyleCap RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a69-f07e-4ca4-af6f-bef486aa4e6f"))
ChartFont : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall put_Background (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Background (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Bold (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Bold (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Color (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Color (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_ColorIndex (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_ColorIndex (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_FontStyle (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_FontStyle (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Italic (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Italic (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Name (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_OutlineFont (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_OutlineFont (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Shadow (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Shadow (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Size (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Size (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Strikethrough (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Strikethrough (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Subscript (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Subscript (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Superscript (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Superscript (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Underline (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Underline (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a57-f07e-4ca4-af6f-bef486aa4e6f"))
ChartCharacters : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Caption (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall put_Caption (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Font (
        /*[out,retval]*/ struct ChartFont * * RHS ) = 0;
      virtual HRESULT __stdcall Insert (
        /*[in]*/ BSTR String,
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Text (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall put_Text (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_PhoneticCharacters (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall put_PhoneticCharacters (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a63-f07e-4ca4-af6f-bef486aa4e6f"))
DataTable : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall put_ShowLegendKey (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_ShowLegendKey (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_HasBorderHorizontal (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_HasBorderHorizontal (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_HasBorderVertical (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_HasBorderVertical (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_HasBorderOutline (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_HasBorderOutline (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * ppLine ) = 0;
      virtual HRESULT __stdcall get_Font (
        /*[out,retval]*/ struct ChartFont * * pfont ) = 0;
      virtual HRESULT __stdcall Select ( ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_AutoScaleFont (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_AutoScaleFont (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a6a-f07e-4ca4-af6f-bef486aa4e6f"))
Gridlines : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * bstr ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * ppparent ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * ppborder ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a6b-f07e-4ca4-af6f-bef486aa4e6f"))
HiLoLines : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
};

struct __declspec(uuid("92d41a6c-f07e-4ca4-af6f-bef486aa4e6f"))
Interior : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall put_Color (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Color (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_ColorIndex (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_ColorIndex (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_InvertIfNegative (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_InvertIfNegative (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Pattern (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Pattern (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_PatternColor (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_PatternColor (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_PatternColorIndex (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_PatternColorIndex (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a54-f07e-4ca4-af6f-bef486aa4e6f"))
AxisTitle : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall put_Caption (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_Caption (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall get_Characters (
        /*[in]*/ VARIANT Start,
        /*[in]*/ VARIANT Length,
        /*[out,retval]*/ struct ChartCharacters * * RHS ) = 0;
      virtual HRESULT __stdcall get_Font (
        /*[out,retval]*/ struct ChartFont * * ppfont ) = 0;
      virtual HRESULT __stdcall put_HorizontalAlignment (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_HorizontalAlignment (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Left (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Left (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall put_Orientation (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Orientation (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Shadow (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_Shadow (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall put_Text (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_Text (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall get_Top (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Top (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall put_VerticalAlignment (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_VerticalAlignment (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_AutoScaleFont (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_AutoScaleFont (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Interior (
        /*[out,retval]*/ struct Interior * * ppinterior ) = 0;
      virtual HRESULT __stdcall get_Fill (
        /*[out,retval]*/ struct ChartFillFormat * * ppinterior ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * ppborder ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_IncludeInLayout (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_IncludeInLayout (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Position (
        /*[out,retval]*/ enum XlChartElementPosition * RHS ) = 0;
      virtual HRESULT __stdcall put_Position (
        /*[in]*/ enum XlChartElementPosition RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * RHS ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_ReadingOrder (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_ReadingOrder (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ double * Height ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ double * Width ) = 0;
      virtual HRESULT __stdcall put_Formula (
        /*[in]*/ BSTR pbstr ) = 0;
      virtual HRESULT __stdcall get_Formula (
        /*[out,retval]*/ BSTR * pbstr ) = 0;
      virtual HRESULT __stdcall put_FormulaR1C1 (
        /*[in]*/ BSTR pbstr ) = 0;
      virtual HRESULT __stdcall get_FormulaR1C1 (
        /*[out,retval]*/ BSTR * pbstr ) = 0;
      virtual HRESULT __stdcall put_FormulaLocal (
        /*[in]*/ BSTR pbstr ) = 0;
      virtual HRESULT __stdcall get_FormulaLocal (
        /*[out,retval]*/ BSTR * pbstr ) = 0;
      virtual HRESULT __stdcall put_FormulaR1C1Local (
        /*[in]*/ BSTR pbstr ) = 0;
      virtual HRESULT __stdcall get_FormulaR1C1Local (
        /*[out,retval]*/ BSTR * pbstr ) = 0;
};

struct __declspec(uuid("92d41a58-f07e-4ca4-af6f-bef486aa4e6f"))
ChartArea : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * ppparent ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * ppborder ) = 0;
      virtual HRESULT __stdcall Clear (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall ClearContents (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall Copy (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall get_Font (
        /*[out,retval]*/ struct ChartFont * * ppfont ) = 0;
      virtual HRESULT __stdcall get_Shadow (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_Shadow (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall ClearFormats (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Height (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_Interior (
        /*[out,retval]*/ struct Interior * * ppinterior ) = 0;
      virtual HRESULT __stdcall get_Fill (
        /*[out,retval]*/ struct ChartFillFormat * * ppFill ) = 0;
      virtual HRESULT __stdcall get_Left (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Left (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_Top (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Top (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Width (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_AutoScaleFont (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall put_AutoScaleFont (
        /*[in]*/ VARIANT pvar ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a5f-f07e-4ca4-af6f-bef486aa4e6f"))
ChartTitle : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall put_Caption (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_Caption (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall get_Characters (
        /*[in]*/ VARIANT Start,
        /*[in]*/ VARIANT Length,
        /*[out,retval]*/ struct ChartCharacters * * RHS ) = 0;
      virtual HRESULT __stdcall get_Font (
        /*[out,retval]*/ struct ChartFont * * ppfont ) = 0;
      virtual HRESULT __stdcall put_HorizontalAlignment (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_HorizontalAlignment (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Left (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Left (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall put_Orientation (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Orientation (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Shadow (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_Shadow (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall put_Text (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_Text (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall get_Top (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Top (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall put_VerticalAlignment (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_VerticalAlignment (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_AutoScaleFont (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_AutoScaleFont (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Interior (
        /*[out,retval]*/ struct Interior * * ppinterior ) = 0;
      virtual HRESULT __stdcall get_Fill (
        /*[out,retval]*/ struct ChartFillFormat * * ppinterior ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * ppborder ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * ppval ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_IncludeInLayout (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_IncludeInLayout (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Position (
        /*[out,retval]*/ enum XlChartElementPosition * RHS ) = 0;
      virtual HRESULT __stdcall put_Position (
        /*[in]*/ enum XlChartElementPosition RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * RHS ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_ReadingOrder (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_ReadingOrder (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ double * Height ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ double * Width ) = 0;
      virtual HRESULT __stdcall put_Formula (
        /*[in]*/ BSTR pbstr ) = 0;
      virtual HRESULT __stdcall get_Formula (
        /*[out,retval]*/ BSTR * pbstr ) = 0;
      virtual HRESULT __stdcall put_FormulaR1C1 (
        /*[in]*/ BSTR pbstr ) = 0;
      virtual HRESULT __stdcall get_FormulaR1C1 (
        /*[out,retval]*/ BSTR * pbstr ) = 0;
      virtual HRESULT __stdcall put_FormulaLocal (
        /*[in]*/ BSTR pbstr ) = 0;
      virtual HRESULT __stdcall get_FormulaLocal (
        /*[out,retval]*/ BSTR * pbstr ) = 0;
      virtual HRESULT __stdcall put_FormulaR1C1Local (
        /*[in]*/ BSTR pbstr ) = 0;
      virtual HRESULT __stdcall get_FormulaR1C1Local (
        /*[out,retval]*/ BSTR * pbstr ) = 0;
};

struct __declspec(uuid("92d41a61-f07e-4ca4-af6f-bef486aa4e6f"))
DataLabel : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * RHS ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Interior (
        /*[out,retval]*/ struct Interior * * RHS ) = 0;
      virtual HRESULT __stdcall get_Fill (
        /*[out,retval]*/ struct ChartFillFormat * * RHS ) = 0;
      virtual HRESULT __stdcall get_Caption (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall put_Caption (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_Characters (
        /*[in]*/ VARIANT Start,
        /*[in]*/ VARIANT Length,
        /*[out,retval]*/ struct ChartCharacters * * RHS ) = 0;
      virtual HRESULT __stdcall get_Font (
        /*[out,retval]*/ struct ChartFont * * RHS ) = 0;
      virtual HRESULT __stdcall get_HorizontalAlignment (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_HorizontalAlignment (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Left (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Left (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_Orientation (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Orientation (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Shadow (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_Shadow (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Text (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall put_Text (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_Top (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Top (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_VerticalAlignment (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_VerticalAlignment (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_ReadingOrder (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_ReadingOrder (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_AutoScaleFont (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_AutoScaleFont (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_AutoText (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_AutoText (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_NumberFormat (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall put_NumberFormat (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_NumberFormatLinked (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_NumberFormatLinked (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_NumberFormatLocal (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_NumberFormatLocal (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_ShowLegendKey (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_ShowLegendKey (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Type (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Position (
        /*[out,retval]*/ enum XlDataLabelPosition * RHS ) = 0;
      virtual HRESULT __stdcall put_Position (
        /*[in]*/ enum XlDataLabelPosition RHS ) = 0;
      virtual HRESULT __stdcall get_ShowSeriesName (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_ShowSeriesName (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_ShowCategoryName (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_ShowCategoryName (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_ShowValue (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_ShowValue (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_ShowPercentage (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_ShowPercentage (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_ShowBubbleSize (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_ShowBubbleSize (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Separator (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Separator (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ double * Height ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ double * Width ) = 0;
      virtual HRESULT __stdcall put_Formula (
        /*[in]*/ BSTR pbstr ) = 0;
      virtual HRESULT __stdcall get_Formula (
        /*[out,retval]*/ BSTR * pbstr ) = 0;
      virtual HRESULT __stdcall put_FormulaR1C1 (
        /*[in]*/ BSTR pbstr ) = 0;
      virtual HRESULT __stdcall get_FormulaR1C1 (
        /*[out,retval]*/ BSTR * pbstr ) = 0;
      virtual HRESULT __stdcall put_FormulaLocal (
        /*[in]*/ BSTR pbstr ) = 0;
      virtual HRESULT __stdcall get_FormulaLocal (
        /*[out,retval]*/ BSTR * pbstr ) = 0;
      virtual HRESULT __stdcall put_FormulaR1C1Local (
        /*[in]*/ BSTR pbstr ) = 0;
      virtual HRESULT __stdcall get_FormulaR1C1Local (
        /*[out,retval]*/ BSTR * pbstr ) = 0;
};

struct __declspec(uuid("92d41a62-f07e-4ca4-af6f-bef486aa4e6f"))
DataLabels : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * RHS ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Interior (
        /*[out,retval]*/ struct Interior * * RHS ) = 0;
      virtual HRESULT __stdcall get_Fill (
        /*[out,retval]*/ struct ChartFillFormat * * RHS ) = 0;
      virtual HRESULT __stdcall get_Font (
        /*[out,retval]*/ struct ChartFont * * RHS ) = 0;
      virtual HRESULT __stdcall get_HorizontalAlignment (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_HorizontalAlignment (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Orientation (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Orientation (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Shadow (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_Shadow (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_VerticalAlignment (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_VerticalAlignment (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_ReadingOrder (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_ReadingOrder (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_AutoScaleFont (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_AutoScaleFont (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_AutoText (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_AutoText (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_NumberFormat (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall put_NumberFormat (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_NumberFormatLinked (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_NumberFormatLinked (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_NumberFormatLocal (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_NumberFormatLocal (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_ShowLegendKey (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_ShowLegendKey (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Type (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Position (
        /*[out,retval]*/ enum XlDataLabelPosition * RHS ) = 0;
      virtual HRESULT __stdcall put_Position (
        /*[in]*/ enum XlDataLabelPosition RHS ) = 0;
      virtual HRESULT __stdcall get_ShowSeriesName (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_ShowSeriesName (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_ShowCategoryName (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_ShowCategoryName (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_ShowValue (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_ShowValue (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_ShowPercentage (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_ShowPercentage (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_ShowBubbleSize (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_ShowBubbleSize (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Separator (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Separator (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct DataLabel * * RHS ) = 0;
      virtual HRESULT __stdcall _NewEnum (
        /*[out,retval]*/ IUnknown * * RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall _Default (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct DataLabel * * RHS ) = 0;
};

struct __declspec(uuid("92d41a64-f07e-4ca4-af6f-bef486aa4e6f"))
DisplayUnitLabel : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall put_Caption (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_Caption (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall get_Characters (
        /*[in]*/ VARIANT Start,
        /*[in]*/ VARIANT Length,
        /*[out,retval]*/ struct ChartCharacters * * RHS ) = 0;
      virtual HRESULT __stdcall get_Font (
        /*[out,retval]*/ struct ChartFont * * ppfont ) = 0;
      virtual HRESULT __stdcall put_HorizontalAlignment (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_HorizontalAlignment (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Left (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Left (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall put_Orientation (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Orientation (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Shadow (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_Shadow (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall put_Text (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_Text (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall get_Top (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Top (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall put_VerticalAlignment (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_VerticalAlignment (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_AutoScaleFont (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_AutoScaleFont (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Interior (
        /*[out,retval]*/ struct Interior * * ppinterior ) = 0;
      virtual HRESULT __stdcall get_Fill (
        /*[out,retval]*/ struct ChartFillFormat * * ppinterior ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * ppborder ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * ppval ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Position (
        /*[out,retval]*/ enum XlChartElementPosition * RHS ) = 0;
      virtual HRESULT __stdcall put_Position (
        /*[in]*/ enum XlChartElementPosition RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * RHS ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_ReadingOrder (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_ReadingOrder (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ double * Height ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ double * Width ) = 0;
      virtual HRESULT __stdcall put_Formula (
        /*[in]*/ BSTR pbstr ) = 0;
      virtual HRESULT __stdcall get_Formula (
        /*[out,retval]*/ BSTR * pbstr ) = 0;
      virtual HRESULT __stdcall put_FormulaR1C1 (
        /*[in]*/ BSTR pbstr ) = 0;
      virtual HRESULT __stdcall get_FormulaR1C1 (
        /*[out,retval]*/ BSTR * pbstr ) = 0;
      virtual HRESULT __stdcall put_FormulaLocal (
        /*[in]*/ BSTR pbstr ) = 0;
      virtual HRESULT __stdcall get_FormulaLocal (
        /*[out,retval]*/ BSTR * pbstr ) = 0;
      virtual HRESULT __stdcall put_FormulaR1C1Local (
        /*[in]*/ BSTR pbstr ) = 0;
      virtual HRESULT __stdcall get_FormulaR1C1Local (
        /*[out,retval]*/ BSTR * pbstr ) = 0;
};

struct __declspec(uuid("92d41a65-f07e-4ca4-af6f-bef486aa4e6f"))
DownBars : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * bstr ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * ppparent ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * ppborder ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall get_Interior (
        /*[out,retval]*/ struct Interior * * ppinterior ) = 0;
      virtual HRESULT __stdcall get_Fill (
        /*[out,retval]*/ struct ChartFillFormat * * ppFill ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a68-f07e-4ca4-af6f-bef486aa4e6f"))
Floor : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * bstr ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * ppparent ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * ppborder ) = 0;
      virtual HRESULT __stdcall ClearFormats (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Interior (
        /*[out,retval]*/ struct Interior * * ppinterior ) = 0;
      virtual HRESULT __stdcall get_Fill (
        /*[out,retval]*/ struct ChartFillFormat * * ppFill ) = 0;
      virtual HRESULT __stdcall get_PictureType (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_PictureType (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall Paste ( ) = 0;
      virtual HRESULT __stdcall get_Thickness (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_Thickness (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a6d-f07e-4ca4-af6f-bef486aa4e6f"))
LeaderLines : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall Select ( ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * ppborder ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a6e-f07e-4ca4-af6f-bef486aa4e6f"))
Legend : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * RHS ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Font (
        /*[out,retval]*/ struct ChartFont * * RHS ) = 0;
      virtual HRESULT __stdcall LegendEntries (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Position (
        /*[out,retval]*/ enum XlLegendPosition * RHS ) = 0;
      virtual HRESULT __stdcall put_Position (
        /*[in]*/ enum XlLegendPosition RHS ) = 0;
      virtual HRESULT __stdcall get_Shadow (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_Shadow (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall Clear (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Height (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_Interior (
        /*[out,retval]*/ struct Interior * * RHS ) = 0;
      virtual HRESULT __stdcall get_Fill (
        /*[out,retval]*/ struct ChartFillFormat * * RHS ) = 0;
      virtual HRESULT __stdcall get_Left (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Left (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_Top (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Top (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Width (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_AutoScaleFont (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_AutoScaleFont (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_IncludeInLayout (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_IncludeInLayout (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a71-f07e-4ca4-af6f-bef486aa4e6f"))
LegendKey : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * RHS ) = 0;
      virtual HRESULT __stdcall ClearFormats (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Interior (
        /*[out,retval]*/ struct Interior * * RHS ) = 0;
      virtual HRESULT __stdcall get_Fill (
        /*[out,retval]*/ struct ChartFillFormat * * RHS ) = 0;
      virtual HRESULT __stdcall get_InvertIfNegative (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_InvertIfNegative (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_MarkerBackgroundColor (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_MarkerBackgroundColor (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_MarkerBackgroundColorIndex (
        /*[out,retval]*/ enum XlColorIndex * RHS ) = 0;
      virtual HRESULT __stdcall put_MarkerBackgroundColorIndex (
        /*[in]*/ enum XlColorIndex RHS ) = 0;
      virtual HRESULT __stdcall get_MarkerForegroundColor (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_MarkerForegroundColor (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_MarkerForegroundColorIndex (
        /*[out,retval]*/ enum XlColorIndex * RHS ) = 0;
      virtual HRESULT __stdcall put_MarkerForegroundColorIndex (
        /*[in]*/ enum XlColorIndex RHS ) = 0;
      virtual HRESULT __stdcall get_MarkerSize (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_MarkerSize (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_MarkerStyle (
        /*[out,retval]*/ enum XlMarkerStyle * RHS ) = 0;
      virtual HRESULT __stdcall put_MarkerStyle (
        /*[in]*/ enum XlMarkerStyle RHS ) = 0;
      virtual HRESULT __stdcall get_PictureType (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_PictureType (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Smooth (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_Smooth (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Left (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall get_Top (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall get_Shadow (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_Shadow (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_PictureUnit2 (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_PictureUnit2 (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_PictureUnit (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_PictureUnit (
        /*[in]*/ long RHS ) = 0;
};

struct __declspec(uuid("92d41a70-f07e-4ca4-af6f-bef486aa4e6f"))
LegendEntry : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Font (
        /*[out,retval]*/ struct ChartFont * * RHS ) = 0;
      virtual HRESULT __stdcall get_Index (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_LegendKey (
        /*[out,retval]*/ struct LegendKey * * RHS ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_AutoScaleFont (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_AutoScaleFont (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Left (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall get_Top (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a6f-f07e-4ca4-af6f-bef486aa4e6f"))
LegendEntries : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct LegendEntry * * RHS ) = 0;
      virtual HRESULT __stdcall _NewEnum (
        /*[out,retval]*/ IUnknown * * RHS ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall _Default (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct LegendEntry * * RHS ) = 0;
};

struct __declspec(uuid("92d41a72-f07e-4ca4-af6f-bef486aa4e6f"))
PlotArea : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * bstr ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * ppparent ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * ppborder ) = 0;
      virtual HRESULT __stdcall ClearFormats (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Height (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_Interior (
        /*[out,retval]*/ struct Interior * * ppinterior ) = 0;
      virtual HRESULT __stdcall get_Fill (
        /*[out,retval]*/ struct ChartFillFormat * * ppFill ) = 0;
      virtual HRESULT __stdcall get_Left (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Left (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_Top (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Top (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Width (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_InsideLeft (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_InsideLeft (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_InsideTop (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_InsideTop (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_InsideWidth (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_InsideWidth (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_InsideHeight (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_InsideHeight (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_Position (
        /*[out,retval]*/ enum XlChartElementPosition * RHS ) = 0;
      virtual HRESULT __stdcall put_Position (
        /*[in]*/ enum XlChartElementPosition RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a75-f07e-4ca4-af6f-bef486aa4e6f"))
Series : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_AxisGroup (
        /*[out,retval]*/ enum XlAxisGroup * RHS ) = 0;
      virtual HRESULT __stdcall put_AxisGroup (
        /*[in]*/ enum XlAxisGroup RHS ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * RHS ) = 0;
      virtual HRESULT __stdcall ClearFormats (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall Copy (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall DataLabels (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall ErrorBar (
        /*[in]*/ enum XlErrorBarDirection Direction,
        /*[in]*/ enum XlErrorBarInclude Include,
        /*[in]*/ enum XlErrorBarType Type,
        /*[in]*/ VARIANT Amount,
        /*[in]*/ VARIANT MinusValues,
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_ErrorBars (
        /*[out,retval]*/ struct ErrorBars * * RHS ) = 0;
      virtual HRESULT __stdcall get_Explosion (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_Explosion (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_Formula (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall put_Formula (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_FormulaLocal (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall put_FormulaLocal (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_FormulaR1C1 (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall put_FormulaR1C1 (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_FormulaR1C1Local (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall put_FormulaR1C1Local (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_HasDataLabels (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_HasDataLabels (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_HasErrorBars (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_HasErrorBars (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Interior (
        /*[out,retval]*/ struct Interior * * RHS ) = 0;
      virtual HRESULT __stdcall get_Fill (
        /*[out,retval]*/ struct ChartFillFormat * * RHS ) = 0;
      virtual HRESULT __stdcall get_InvertIfNegative (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_InvertIfNegative (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_MarkerBackgroundColor (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_MarkerBackgroundColor (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_MarkerBackgroundColorIndex (
        /*[out,retval]*/ enum XlColorIndex * RHS ) = 0;
      virtual HRESULT __stdcall put_MarkerBackgroundColorIndex (
        /*[in]*/ enum XlColorIndex RHS ) = 0;
      virtual HRESULT __stdcall get_MarkerForegroundColor (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_MarkerForegroundColor (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_MarkerForegroundColorIndex (
        /*[out,retval]*/ enum XlColorIndex * RHS ) = 0;
      virtual HRESULT __stdcall put_MarkerForegroundColorIndex (
        /*[in]*/ enum XlColorIndex RHS ) = 0;
      virtual HRESULT __stdcall get_MarkerSize (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_MarkerSize (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_MarkerStyle (
        /*[out,retval]*/ enum XlMarkerStyle * RHS ) = 0;
      virtual HRESULT __stdcall put_MarkerStyle (
        /*[in]*/ enum XlMarkerStyle RHS ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall put_Name (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall Paste (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_PictureType (
        /*[out,retval]*/ enum XlChartPictureType * RHS ) = 0;
      virtual HRESULT __stdcall put_PictureType (
        /*[in]*/ enum XlChartPictureType RHS ) = 0;
      virtual HRESULT __stdcall get_PlotOrder (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_PlotOrder (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall Points (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Smooth (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_Smooth (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall Trendlines (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_Type (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_ChartType (
        /*[out,retval]*/ enum Office2010::XlChartType * RHS ) = 0;
      virtual HRESULT __stdcall put_ChartType (
        /*[in]*/ enum Office2010::XlChartType RHS ) = 0;
      virtual HRESULT __stdcall ApplyCustomType (
        /*[in]*/ enum Office2010::XlChartType ChartType ) = 0;
      virtual HRESULT __stdcall get_Values (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Values (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_XValues (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_XValues (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_BubbleSizes (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_BubbleSizes (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_BarShape (
        /*[out,retval]*/ enum XlBarShape * RHS ) = 0;
      virtual HRESULT __stdcall put_BarShape (
        /*[in]*/ enum XlBarShape RHS ) = 0;
      virtual HRESULT __stdcall get_ApplyPictToSides (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_ApplyPictToSides (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_ApplyPictToFront (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_ApplyPictToFront (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_ApplyPictToEnd (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_ApplyPictToEnd (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Has3DEffect (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_Has3DEffect (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Shadow (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_Shadow (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_HasLeaderLines (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_HasLeaderLines (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_LeaderLines (
        /*[out,retval]*/ struct LeaderLines * * RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_PictureUnit2 (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_PictureUnit2 (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall _ApplyDataLabels (
        /*[in]*/ enum XlDataLabelsType Type,
        /*[in]*/ VARIANT LegendKey,
        /*[in]*/ VARIANT AutoText,
        /*[in]*/ VARIANT HasLeaderLines,
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall ApplyDataLabels (
        /*[in]*/ enum XlDataLabelsType Type,
        /*[in]*/ VARIANT LegendKey,
        /*[in]*/ VARIANT AutoText,
        /*[in]*/ VARIANT HasLeaderLines,
        /*[in]*/ VARIANT ShowSeriesName,
        /*[in]*/ VARIANT ShowCategoryName,
        /*[in]*/ VARIANT ShowValue,
        /*[in]*/ VARIANT ShowPercentage,
        /*[in]*/ VARIANT ShowBubbleSize,
        /*[in]*/ VARIANT Separator,
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_PictureUnit (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_PictureUnit (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_PlotColorIndex (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_InvertColor (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_InvertColor (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_InvertColorIndex (
        /*[out,retval]*/ enum XlColorIndex * RHS ) = 0;
      virtual HRESULT __stdcall put_InvertColorIndex (
        /*[in]*/ enum XlColorIndex RHS ) = 0;
};

struct __declspec(uuid("92d41a76-f07e-4ca4-af6f-bef486aa4e6f"))
SeriesCollection : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall Extend (
        /*[in]*/ VARIANT Source,
        /*[in]*/ VARIANT Rowcol,
        /*[in]*/ VARIANT CategoryLabels,
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct Series * * RHS ) = 0;
      virtual HRESULT __stdcall _NewEnum (
        /*[out,retval]*/ IUnknown * * RHS ) = 0;
      virtual HRESULT __stdcall NewSeries (
        /*[out,retval]*/ struct Series * * RHS ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall Add (
        /*[in]*/ VARIANT Source,
        /*[in]*/ enum XlRowCol Rowcol,
        /*[in]*/ VARIANT SeriesLabels,
        /*[in]*/ VARIANT CategoryLabels,
        /*[in]*/ VARIANT Replace,
        /*[out,retval]*/ struct Series * * RHS ) = 0;
      virtual HRESULT __stdcall _Default (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct Series * * RHS ) = 0;
};

struct __declspec(uuid("92d41a77-f07e-4ca4-af6f-bef486aa4e6f"))
SeriesLines : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * bstr ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * ppparent ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * ppborder ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a78-f07e-4ca4-af6f-bef486aa4e6f"))
TickLabels : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * ppval ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Font (
        /*[out,retval]*/ struct ChartFont * * ppval ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall get_NumberFormat (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall put_NumberFormat (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_NumberFormatLinked (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_NumberFormatLinked (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_NumberFormatLocal (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_NumberFormatLocal (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Orientation (
        /*[out,retval]*/ enum XlTickLabelOrientation * RHS ) = 0;
      virtual HRESULT __stdcall put_Orientation (
        /*[in]*/ enum XlTickLabelOrientation RHS ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_AutoScaleFont (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_AutoScaleFont (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Depth (
        /*[out,retval]*/ int * RHS ) = 0;
      virtual HRESULT __stdcall get_Offset (
        /*[out,retval]*/ int * RHS ) = 0;
      virtual HRESULT __stdcall put_Offset (
        /*[in]*/ int RHS ) = 0;
      virtual HRESULT __stdcall get_MultiLevel (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_MultiLevel (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Alignment (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_Alignment (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_ReadingOrder (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_ReadingOrder (
        /*[in]*/ long RHS ) = 0;
};

struct __declspec(uuid("92d41a53-f07e-4ca4-af6f-bef486aa4e6f"))
Axis : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_AxisBetweenCategories (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_AxisBetweenCategories (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_AxisGroup (
        /*[out,retval]*/ enum XlAxisGroup * RHS ) = 0;
      virtual HRESULT __stdcall get_AxisTitle (
        /*[out,retval]*/ struct AxisTitle * * RHS ) = 0;
      virtual HRESULT __stdcall get_CategoryNames (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_CategoryNames (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Crosses (
        /*[out,retval]*/ enum XlAxisCrosses * RHS ) = 0;
      virtual HRESULT __stdcall put_Crosses (
        /*[in]*/ enum XlAxisCrosses RHS ) = 0;
      virtual HRESULT __stdcall get_CrossesAt (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_CrossesAt (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_HasMajorGridlines (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_HasMajorGridlines (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_HasMinorGridlines (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_HasMinorGridlines (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_HasTitle (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_HasTitle (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_MajorGridlines (
        /*[out,retval]*/ struct Gridlines * * RHS ) = 0;
      virtual HRESULT __stdcall get_MajorTickMark (
        /*[out,retval]*/ enum XlTickMark * RHS ) = 0;
      virtual HRESULT __stdcall put_MajorTickMark (
        /*[in]*/ enum XlTickMark RHS ) = 0;
      virtual HRESULT __stdcall get_MajorUnit (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_MajorUnit (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_LogBase (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_LogBase (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_TickLabelSpacingIsAuto (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_TickLabelSpacingIsAuto (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_MajorUnitIsAuto (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_MajorUnitIsAuto (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_MaximumScale (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_MaximumScale (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_MaximumScaleIsAuto (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_MaximumScaleIsAuto (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_MinimumScale (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_MinimumScale (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_MinimumScaleIsAuto (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_MinimumScaleIsAuto (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_MinorGridlines (
        /*[out,retval]*/ struct Gridlines * * RHS ) = 0;
      virtual HRESULT __stdcall get_MinorTickMark (
        /*[out,retval]*/ enum XlTickMark * RHS ) = 0;
      virtual HRESULT __stdcall put_MinorTickMark (
        /*[in]*/ enum XlTickMark RHS ) = 0;
      virtual HRESULT __stdcall get_MinorUnit (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_MinorUnit (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_MinorUnitIsAuto (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_MinorUnitIsAuto (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_ReversePlotOrder (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_ReversePlotOrder (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_ScaleType (
        /*[out,retval]*/ enum XlScaleType * RHS ) = 0;
      virtual HRESULT __stdcall put_ScaleType (
        /*[in]*/ enum XlScaleType RHS ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_TickLabelPosition (
        /*[out,retval]*/ enum XlTickLabelPosition * RHS ) = 0;
      virtual HRESULT __stdcall put_TickLabelPosition (
        /*[in]*/ enum XlTickLabelPosition RHS ) = 0;
      virtual HRESULT __stdcall get_TickLabels (
        /*[out,retval]*/ struct TickLabels * * RHS ) = 0;
      virtual HRESULT __stdcall get_TickLabelSpacing (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_TickLabelSpacing (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_TickMarkSpacing (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_TickMarkSpacing (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum XlAxisType * RHS ) = 0;
      virtual HRESULT __stdcall put_Type (
        /*[in]*/ enum XlAxisType RHS ) = 0;
      virtual HRESULT __stdcall get_BaseUnit (
        /*[out,retval]*/ enum XlTimeUnit * RHS ) = 0;
      virtual HRESULT __stdcall put_BaseUnit (
        /*[in]*/ enum XlTimeUnit RHS ) = 0;
      virtual HRESULT __stdcall get_BaseUnitIsAuto (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_BaseUnitIsAuto (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_MajorUnitScale (
        /*[out,retval]*/ enum XlTimeUnit * RHS ) = 0;
      virtual HRESULT __stdcall put_MajorUnitScale (
        /*[in]*/ enum XlTimeUnit RHS ) = 0;
      virtual HRESULT __stdcall get_MinorUnitScale (
        /*[out,retval]*/ enum XlTimeUnit * RHS ) = 0;
      virtual HRESULT __stdcall put_MinorUnitScale (
        /*[in]*/ enum XlTimeUnit RHS ) = 0;
      virtual HRESULT __stdcall get_CategoryType (
        /*[out,retval]*/ enum XlCategoryType * RHS ) = 0;
      virtual HRESULT __stdcall put_CategoryType (
        /*[in]*/ enum XlCategoryType RHS ) = 0;
      virtual HRESULT __stdcall get_Left (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall get_Top (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall get_DisplayUnit (
        /*[out,retval]*/ enum XlDisplayUnit * RHS ) = 0;
      virtual HRESULT __stdcall put_DisplayUnit (
        /*[in]*/ enum XlDisplayUnit RHS ) = 0;
      virtual HRESULT __stdcall get_DisplayUnitCustom (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_DisplayUnitCustom (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_HasDisplayUnitLabel (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_HasDisplayUnitLabel (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_DisplayUnitLabel (
        /*[out,retval]*/ struct DisplayUnitLabel * * RHS ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * ppborder ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a52-f07e-4ca4-af6f-bef486aa4e6f"))
Axes : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ enum XlAxisType Type,
        /*[in]*/ enum XlAxisGroup AxisGroup,
        /*[out,retval]*/ struct Axis * * RHS ) = 0;
      virtual HRESULT __stdcall _NewEnum (
        /*[out,retval]*/ IUnknown * * RHS ) = 0;
      virtual HRESULT __stdcall _Default (
        /*[in]*/ enum XlAxisType Type,
        /*[in]*/ enum XlAxisGroup AxisGroup,
        /*[out,retval]*/ struct Axis * * RHS ) = 0;
};

struct __declspec(uuid("92d41a79-f07e-4ca4-af6f-bef486aa4e6f"))
Trendline : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * RHS ) = 0;
      virtual HRESULT __stdcall ClearFormats (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_DataLabel (
        /*[out,retval]*/ struct DataLabel * * RHS ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_DisplayEquation (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_DisplayEquation (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_DisplayRSquared (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_DisplayRSquared (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Index (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_Intercept (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Intercept (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_InterceptIsAuto (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_InterceptIsAuto (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall put_Name (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_NameIsAuto (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_NameIsAuto (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Order (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_Order (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_Period (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_Period (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum XlTrendlineType * RHS ) = 0;
      virtual HRESULT __stdcall put_Type (
        /*[in]*/ enum XlTrendlineType RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Backward2 (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Backward2 (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_Forward2 (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_Forward2 (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Backward (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_Backward (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_Forward (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_Forward (
        /*[in]*/ long RHS ) = 0;
};

struct __declspec(uuid("92d41a7a-f07e-4ca4-af6f-bef486aa4e6f"))
Trendlines : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall Add (
        /*[in]*/ enum XlTrendlineType Type,
        /*[in]*/ VARIANT Order,
        /*[in]*/ VARIANT Period,
        /*[in]*/ VARIANT Forward,
        /*[in]*/ VARIANT Backward,
        /*[in]*/ VARIANT Intercept,
        /*[in]*/ VARIANT DisplayEquation,
        /*[in]*/ VARIANT DisplayRSquared,
        /*[in]*/ VARIANT Name,
        /*[out,retval]*/ struct Trendline * * RHS ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct Trendline * * RHS ) = 0;
      virtual HRESULT __stdcall _NewEnum (
        /*[out,retval]*/ IUnknown * * RHS ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall _Default (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct Trendline * * RHS ) = 0;
};

struct __declspec(uuid("92d41a7b-f07e-4ca4-af6f-bef486aa4e6f"))
UpBars : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * bstr ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * ppparent ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * ppborder ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall get_Interior (
        /*[out,retval]*/ struct Interior * * ppinterior ) = 0;
      virtual HRESULT __stdcall get_Fill (
        /*[out,retval]*/ struct ChartFillFormat * * ppFill ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a5d-f07e-4ca4-af6f-bef486aa4e6f"))
ChartGroup : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_DownBars (
        /*[out,retval]*/ struct DownBars * * ppdownbars ) = 0;
      virtual HRESULT __stdcall get_DropLines (
        /*[out,retval]*/ struct DropLines * * ppdroplines ) = 0;
      virtual HRESULT __stdcall put_HasDropLines (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_HasDropLines (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_HasHiLoLines (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_HasHiLoLines (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_HasRadarAxisLabels (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_HasRadarAxisLabels (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_HasSeriesLines (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_HasSeriesLines (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_HasUpDownBars (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_HasUpDownBars (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall get_HiLoLines (
        /*[out,retval]*/ struct HiLoLines * * ppHiLoLines ) = 0;
      virtual HRESULT __stdcall SeriesCollection (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ IDispatch * * ppSeriesCollection ) = 0;
      virtual HRESULT __stdcall get_SeriesLines (
        /*[out,retval]*/ struct SeriesLines * * ppSeriesLines ) = 0;
      virtual HRESULT __stdcall get_UpBars (
        /*[out,retval]*/ struct UpBars * * ppUpBars ) = 0;
      virtual HRESULT __stdcall put_VaryByCategories (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_VaryByCategories (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall get_SizeRepresents (
        /*[out,retval]*/ enum XlSizeRepresents * RHS ) = 0;
      virtual HRESULT __stdcall put_SizeRepresents (
        /*[in]*/ enum XlSizeRepresents RHS ) = 0;
      virtual HRESULT __stdcall put_ShowNegativeBubbles (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_ShowNegativeBubbles (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_SplitType (
        /*[in]*/ enum XlChartSplitType RHS ) = 0;
      virtual HRESULT __stdcall get_SplitType (
        /*[out,retval]*/ enum XlChartSplitType * RHS ) = 0;
      virtual HRESULT __stdcall get_SplitValue (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_SplitValue (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Has3DShading (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_Has3DShading (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_AxisGroup (
        /*[out,retval]*/ enum XlAxisGroup * RHS ) = 0;
      virtual HRESULT __stdcall put_AxisGroup (
        /*[in]*/ enum XlAxisGroup RHS ) = 0;
      virtual HRESULT __stdcall get_BubbleScale (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_BubbleScale (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_DoughnutHoleSize (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_DoughnutHoleSize (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_FirstSliceAngle (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_FirstSliceAngle (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_GapWidth (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_GapWidth (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_Index (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_Overlap (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_Overlap (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_RadarAxisLabels (
        /*[out,retval]*/ struct TickLabels * * RHS ) = 0;
      virtual HRESULT __stdcall get_Subtype (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_Subtype (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_Type (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_SecondPlotSize (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_SecondPlotSize (
        /*[in]*/ long RHS ) = 0;
};

struct __declspec(uuid("92d41a5e-f07e-4ca4-af6f-bef486aa4e6f"))
ChartGroups : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct ChartGroup * * RHS ) = 0;
      virtual HRESULT __stdcall _NewEnum (
        /*[out,retval]*/ IUnknown * * RHS ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

struct __declspec(uuid("92d41a7c-f07e-4ca4-af6f-bef486aa4e6f"))
Walls : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * bstr ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * ppparent ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * ppborder ) = 0;
      virtual HRESULT __stdcall ClearFormats (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall get_Interior (
        /*[out,retval]*/ struct Interior * * ppinterior ) = 0;
      virtual HRESULT __stdcall get_Fill (
        /*[out,retval]*/ struct ChartFillFormat * * ppFill ) = 0;
      virtual HRESULT __stdcall get_PictureType (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall put_PictureType (
        /*[in]*/ VARIANT pvar ) = 0;
      virtual HRESULT __stdcall Paste ( ) = 0;
      virtual HRESULT __stdcall get_PictureUnit (
        /*[out,retval]*/ VARIANT * pvar ) = 0;
      virtual HRESULT __stdcall put_PictureUnit (
        /*[in]*/ VARIANT pvar ) = 0;
      virtual HRESULT __stdcall get_Thickness (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_Thickness (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
};

enum __declspec(uuid("cb5dd948-aab3-405f-9f29-79468f1f5971"))
PpResampleMediaProfile
{
    ppResampleMediaProfileCustom = 1,
    ppResampleMediaProfileSmall = 2,
    ppResampleMediaProfileSmaller = 3,
    ppResampleMediaProfileSmallest = 4
};

enum __declspec(uuid("b44003e4-31c5-46da-9fba-985a62352afd"))
PpMediaTaskStatus
{
    ppMediaTaskStatusNone = 0,
    ppMediaTaskStatusInProgress = 1,
    ppMediaTaskStatusQueued = 2,
    ppMediaTaskStatusDone = 3,
    ppMediaTaskStatusFailed = 4
};

struct __declspec(uuid("ba72e551-4ff5-48f4-8215-5505f990966f"))
SectionProperties : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ long * Count ) = 0;
      virtual HRESULT __stdcall Name (
        /*[in]*/ int sectionIndex,
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall Rename (
        /*[in]*/ int sectionIndex,
        /*[in]*/ BSTR sectionName ) = 0;
      virtual HRESULT __stdcall SlidesCount (
        /*[in]*/ int sectionIndex,
        /*[out,retval]*/ long * SlidesCount ) = 0;
      virtual HRESULT __stdcall FirstSlide (
        /*[in]*/ int sectionIndex,
        /*[out,retval]*/ int * FirstSlide ) = 0;
      virtual HRESULT __stdcall AddBeforeSlide (
        /*[in]*/ int SlideIndex,
        /*[in]*/ BSTR sectionName,
        /*[out,retval]*/ int * BeforeSlide ) = 0;
      virtual HRESULT __stdcall AddSection (
        /*[in]*/ int sectionIndex,
        /*[in]*/ VARIANT sectionName,
        /*[out,retval]*/ int * Section ) = 0;
      virtual HRESULT __stdcall Move (
        /*[in]*/ int sectionIndex,
        /*[in]*/ int toPos ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[in]*/ int sectionIndex,
        /*[in]*/ VARIANT_BOOL deleteSlides ) = 0;
      virtual HRESULT __stdcall SectionID (
        /*[in]*/ int sectionIndex,
        /*[out,retval]*/ BSTR * SectionID ) = 0;
};

enum __declspec(uuid("1ab5ac71-b868-45ce-94dd-42b327b8bc0f"))
PpPlayerState
{
    ppPlaying = 0,
    ppPaused = 1,
    ppStopped = 2,
    ppNotReady = 3
};

struct __declspec(uuid("ba72e552-4ff5-48f4-8215-5505f990966f"))
Player : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Play ( ) = 0;
      virtual HRESULT __stdcall Pause ( ) = 0;
      virtual HRESULT __stdcall Stop ( ) = 0;
      virtual HRESULT __stdcall GoToNextBookmark ( ) = 0;
      virtual HRESULT __stdcall GoToPreviousBookmark ( ) = 0;
      virtual HRESULT __stdcall get_CurrentPosition (
        /*[out,retval]*/ long * CurrentPosition ) = 0;
      virtual HRESULT __stdcall put_CurrentPosition (
        /*[in]*/ long CurrentPosition ) = 0;
      virtual HRESULT __stdcall get_State (
        /*[out,retval]*/ enum PpPlayerState * State ) = 0;
};

struct __declspec(uuid("91493458-5a91-11cf-8700-00aa0060263b"))
View : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum PpViewType * Type ) = 0;
      virtual HRESULT __stdcall get_Zoom (
        /*[out,retval]*/ int * Zoom ) = 0;
      virtual HRESULT __stdcall put_Zoom (
        /*[in]*/ int Zoom ) = 0;
      virtual HRESULT __stdcall Paste ( ) = 0;
      virtual HRESULT __stdcall get_Slide (
        /*[out,retval]*/ IDispatch * * Slide ) = 0;
      virtual HRESULT __stdcall put_Slide (
        /*[in]*/ IDispatch * Slide ) = 0;
      virtual HRESULT __stdcall GotoSlide (
        /*[in]*/ int Index ) = 0;
      virtual HRESULT __stdcall get_DisplaySlideMiniature (
        /*[out,retval]*/ enum Office2010::MsoTriState * DisplaySlideMiniature ) = 0;
      virtual HRESULT __stdcall put_DisplaySlideMiniature (
        /*[in]*/ enum Office2010::MsoTriState DisplaySlideMiniature ) = 0;
      virtual HRESULT __stdcall get_ZoomToFit (
        /*[out,retval]*/ enum Office2010::MsoTriState * ZoomToFit ) = 0;
      virtual HRESULT __stdcall put_ZoomToFit (
        /*[in]*/ enum Office2010::MsoTriState ZoomToFit ) = 0;
      virtual HRESULT __stdcall PasteSpecial (
        /*[in]*/ enum PpPasteDataType DataType,
        /*[in]*/ enum Office2010::MsoTriState DisplayAsIcon,
        /*[in]*/ BSTR IconFileName,
        /*[in]*/ int IconIndex,
        /*[in]*/ BSTR IconLabel,
        /*[in]*/ enum Office2010::MsoTriState Link ) = 0;
      virtual HRESULT __stdcall get_PrintOptions (
        /*[out,retval]*/ struct PrintOptions * * PrintOptions ) = 0;
      virtual HRESULT __stdcall PrintOut (
        /*[in]*/ int From,
        /*[in]*/ int To,
        /*[in]*/ BSTR PrintToFile,
        /*[in]*/ int Copies,
        /*[in]*/ enum Office2010::MsoTriState Collate ) = 0;
      virtual HRESULT __stdcall Player (
        /*[in]*/ VARIANT ShapeId,
        /*[out,retval]*/ struct Player * * Player ) = 0;
      virtual HRESULT __stdcall get_MediaControlsVisible (
        /*[out,retval]*/ enum Office2010::MsoTriState * MediaControlsVisible ) = 0;
      virtual HRESULT __stdcall get_MediaControlsLeft (
        /*[out,retval]*/ float * MediaControlsLeft ) = 0;
      virtual HRESULT __stdcall get_MediaControlsTop (
        /*[out,retval]*/ float * MediaControlsTop ) = 0;
      virtual HRESULT __stdcall get_MediaControlsWidth (
        /*[out,retval]*/ float * MediaControlsWidth ) = 0;
      virtual HRESULT __stdcall get_MediaControlsHeight (
        /*[out,retval]*/ float * MediaControlsHeight ) = 0;
};

struct __declspec(uuid("91493459-5a91-11cf-8700-00aa0060263b"))
SlideShowView : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Zoom (
        /*[out,retval]*/ int * Zoom ) = 0;
      virtual HRESULT __stdcall get_Slide (
        /*[out,retval]*/ struct _Slide * * Slide ) = 0;
      virtual HRESULT __stdcall get_PointerType (
        /*[out,retval]*/ enum PpSlideShowPointerType * PointerType ) = 0;
      virtual HRESULT __stdcall put_PointerType (
        /*[in]*/ enum PpSlideShowPointerType PointerType ) = 0;
      virtual HRESULT __stdcall get_State (
        /*[out,retval]*/ enum PpSlideShowState * State ) = 0;
      virtual HRESULT __stdcall put_State (
        /*[in]*/ enum PpSlideShowState State ) = 0;
      virtual HRESULT __stdcall get_AcceleratorsEnabled (
        /*[out,retval]*/ enum Office2010::MsoTriState * AcceleratorsEnabled ) = 0;
      virtual HRESULT __stdcall put_AcceleratorsEnabled (
        /*[in]*/ enum Office2010::MsoTriState AcceleratorsEnabled ) = 0;
      virtual HRESULT __stdcall get_PresentationElapsedTime (
        /*[out,retval]*/ float * PresentationElapsedTime ) = 0;
      virtual HRESULT __stdcall get_SlideElapsedTime (
        /*[out,retval]*/ float * SlideElapsedTime ) = 0;
      virtual HRESULT __stdcall put_SlideElapsedTime (
        /*[in]*/ float SlideElapsedTime ) = 0;
      virtual HRESULT __stdcall get_LastSlideViewed (
        /*[out,retval]*/ struct _Slide * * LastSlideViewed ) = 0;
      virtual HRESULT __stdcall get_AdvanceMode (
        /*[out,retval]*/ enum PpSlideShowAdvanceMode * AdvanceMode ) = 0;
      virtual HRESULT __stdcall get_PointerColor (
        /*[out,retval]*/ struct ColorFormat * * PointerColor ) = 0;
      virtual HRESULT __stdcall get_IsNamedShow (
        /*[out,retval]*/ enum Office2010::MsoTriState * IsNamedShow ) = 0;
      virtual HRESULT __stdcall get_SlideShowName (
        /*[out,retval]*/ BSTR * SlideShowName ) = 0;
      virtual HRESULT __stdcall DrawLine (
        /*[in]*/ float BeginX,
        /*[in]*/ float BeginY,
        /*[in]*/ float EndX,
        /*[in]*/ float EndY ) = 0;
      virtual HRESULT __stdcall EraseDrawing ( ) = 0;
      virtual HRESULT __stdcall First ( ) = 0;
      virtual HRESULT __stdcall Last ( ) = 0;
      virtual HRESULT __stdcall Next ( ) = 0;
      virtual HRESULT __stdcall Previous ( ) = 0;
      virtual HRESULT __stdcall GotoSlide (
        /*[in]*/ int Index,
        /*[in]*/ enum Office2010::MsoTriState ResetSlide ) = 0;
      virtual HRESULT __stdcall GotoNamedShow (
        /*[in]*/ BSTR SlideShowName ) = 0;
      virtual HRESULT __stdcall EndNamedShow ( ) = 0;
      virtual HRESULT __stdcall ResetSlideTime ( ) = 0;
      virtual HRESULT __stdcall Exit ( ) = 0;
      virtual HRESULT __stdcall InstallTracker (
        /*[in]*/ struct MouseTracker * pTracker,
        /*[in]*/ enum Office2010::MsoTriState Presenter ) = 0;
      virtual HRESULT __stdcall get_CurrentShowPosition (
        /*[out,retval]*/ int * CurrentShowPosition ) = 0;
      virtual HRESULT __stdcall GotoClick (
        /*[in]*/ int Index ) = 0;
      virtual HRESULT __stdcall GetClickIndex (
        /*[out,retval]*/ int * GetClickIndex ) = 0;
      virtual HRESULT __stdcall GetClickCount (
        /*[out,retval]*/ int * GetClickCount ) = 0;
      virtual HRESULT __stdcall FirstAnimationIsAutomatic (
        /*[out,retval]*/ VARIANT_BOOL * FirstAnimationIsAutomatic ) = 0;
      virtual HRESULT __stdcall Player (
        /*[in]*/ VARIANT ShapeId,
        /*[out,retval]*/ struct Player * * Player ) = 0;
      virtual HRESULT __stdcall get_MediaControlsVisible (
        /*[out,retval]*/ enum Office2010::MsoTriState * MediaControlsVisible ) = 0;
      virtual HRESULT __stdcall get_MediaControlsLeft (
        /*[out,retval]*/ float * MediaControlsLeft ) = 0;
      virtual HRESULT __stdcall get_MediaControlsTop (
        /*[out,retval]*/ float * MediaControlsTop ) = 0;
      virtual HRESULT __stdcall get_MediaControlsWidth (
        /*[out,retval]*/ float * MediaControlsWidth ) = 0;
      virtual HRESULT __stdcall get_MediaControlsHeight (
        /*[out,retval]*/ float * MediaControlsHeight ) = 0;
};

struct __declspec(uuid("91493453-5a91-11cf-8700-00aa0060263b"))
SlideShowWindow : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_View (
        /*[out,retval]*/ struct SlideShowView * * View ) = 0;
      virtual HRESULT __stdcall get_Presentation (
        /*[out,retval]*/ struct _Presentation * * Presentation ) = 0;
      virtual HRESULT __stdcall get_IsFullScreen (
        /*[out,retval]*/ enum Office2010::MsoTriState * IsFullScreen ) = 0;
      virtual HRESULT __stdcall get_Left (
        /*[out,retval]*/ float * Left ) = 0;
      virtual HRESULT __stdcall put_Left (
        /*[in]*/ float Left ) = 0;
      virtual HRESULT __stdcall get_Top (
        /*[out,retval]*/ float * Top ) = 0;
      virtual HRESULT __stdcall put_Top (
        /*[in]*/ float Top ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ float * Width ) = 0;
      virtual HRESULT __stdcall put_Width (
        /*[in]*/ float Width ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ float * Height ) = 0;
      virtual HRESULT __stdcall put_Height (
        /*[in]*/ float Height ) = 0;
      virtual HRESULT __stdcall get_HWND (
        /*[out,retval]*/ long * HWND ) = 0;
      virtual HRESULT __stdcall get_Active (
        /*[out,retval]*/ enum Office2010::MsoTriState * Active ) = 0;
      virtual HRESULT __stdcall Activate ( ) = 0;
};

struct __declspec(uuid("91493456-5a91-11cf-8700-00aa0060263b"))
SlideShowWindows : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct SlideShowWindow * * Item ) = 0;
};

struct __declspec(uuid("9149345a-5a91-11cf-8700-00aa0060263b"))
SlideShowSettings : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_PointerColor (
        /*[out,retval]*/ struct ColorFormat * * PointerColor ) = 0;
      virtual HRESULT __stdcall get_NamedSlideShows (
        /*[out,retval]*/ struct NamedSlideShows * * NamedSlideShows ) = 0;
      virtual HRESULT __stdcall get_StartingSlide (
        /*[out,retval]*/ int * StartingSlide ) = 0;
      virtual HRESULT __stdcall put_StartingSlide (
        /*[in]*/ int StartingSlide ) = 0;
      virtual HRESULT __stdcall get_EndingSlide (
        /*[out,retval]*/ int * EndingSlide ) = 0;
      virtual HRESULT __stdcall put_EndingSlide (
        /*[in]*/ int EndingSlide ) = 0;
      virtual HRESULT __stdcall get_AdvanceMode (
        /*[out,retval]*/ enum PpSlideShowAdvanceMode * AdvanceMode ) = 0;
      virtual HRESULT __stdcall put_AdvanceMode (
        /*[in]*/ enum PpSlideShowAdvanceMode AdvanceMode ) = 0;
      virtual HRESULT __stdcall Run (
        /*[out,retval]*/ struct SlideShowWindow * * Run ) = 0;
      virtual HRESULT __stdcall get_LoopUntilStopped (
        /*[out,retval]*/ enum Office2010::MsoTriState * LoopUntilStopped ) = 0;
      virtual HRESULT __stdcall put_LoopUntilStopped (
        /*[in]*/ enum Office2010::MsoTriState LoopUntilStopped ) = 0;
      virtual HRESULT __stdcall get_ShowType (
        /*[out,retval]*/ enum PpSlideShowType * ShowType ) = 0;
      virtual HRESULT __stdcall put_ShowType (
        /*[in]*/ enum PpSlideShowType ShowType ) = 0;
      virtual HRESULT __stdcall get_ShowWithNarration (
        /*[out,retval]*/ enum Office2010::MsoTriState * ShowWithNarration ) = 0;
      virtual HRESULT __stdcall put_ShowWithNarration (
        /*[in]*/ enum Office2010::MsoTriState ShowWithNarration ) = 0;
      virtual HRESULT __stdcall get_ShowWithAnimation (
        /*[out,retval]*/ enum Office2010::MsoTriState * ShowWithAnimation ) = 0;
      virtual HRESULT __stdcall put_ShowWithAnimation (
        /*[in]*/ enum Office2010::MsoTriState ShowWithAnimation ) = 0;
      virtual HRESULT __stdcall get_SlideShowName (
        /*[out,retval]*/ BSTR * SlideShowName ) = 0;
      virtual HRESULT __stdcall put_SlideShowName (
        /*[in]*/ BSTR SlideShowName ) = 0;
      virtual HRESULT __stdcall get_RangeType (
        /*[out,retval]*/ enum PpSlideShowRangeType * RangeType ) = 0;
      virtual HRESULT __stdcall put_RangeType (
        /*[in]*/ enum PpSlideShowRangeType RangeType ) = 0;
      virtual HRESULT __stdcall get_ShowScrollbar (
        /*[out,retval]*/ enum Office2010::MsoTriState * ShowScrollbar ) = 0;
      virtual HRESULT __stdcall put_ShowScrollbar (
        /*[in]*/ enum Office2010::MsoTriState ShowScrollbar ) = 0;
      virtual HRESULT __stdcall get_ShowPresenterView (
        /*[out,retval]*/ enum Office2010::MsoTriState * ShowPresenterView ) = 0;
      virtual HRESULT __stdcall put_ShowPresenterView (
        /*[in]*/ enum Office2010::MsoTriState ShowPresenterView ) = 0;
      virtual HRESULT __stdcall get_ShowMediaControls (
        /*[out,retval]*/ enum Office2010::MsoTriState * ShowMediaControls ) = 0;
      virtual HRESULT __stdcall put_ShowMediaControls (
        /*[in]*/ enum Office2010::MsoTriState ShowMediaControls ) = 0;
};

struct __declspec(uuid("ba72e555-4ff5-48f4-8215-5505f990966f"))
MediaBookmark : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Index (
        /*[out,retval]*/ int * Index ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall get_Position (
        /*[out,retval]*/ long * Position ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
};

struct __declspec(uuid("ba72e556-4ff5-48f4-8215-5505f990966f"))
MediaBookmarks : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct MediaBookmark * * Item ) = 0;
      virtual HRESULT __stdcall Add (
        /*[in]*/ long Position,
        /*[in]*/ BSTR Name,
        /*[out,retval]*/ struct MediaBookmark * * Add ) = 0;
};

struct __declspec(uuid("ba72e550-4ff5-48f4-8215-5505f990966f"))
MediaFormat : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Volume (
        /*[out,retval]*/ float * Volume ) = 0;
      virtual HRESULT __stdcall put_Volume (
        /*[in]*/ float Volume ) = 0;
      virtual HRESULT __stdcall get_Muted (
        /*[out,retval]*/ VARIANT_BOOL * Muted ) = 0;
      virtual HRESULT __stdcall put_Muted (
        /*[in]*/ VARIANT_BOOL Muted ) = 0;
      virtual HRESULT __stdcall get_Length (
        /*[out,retval]*/ long * Length ) = 0;
      virtual HRESULT __stdcall get_StartPoint (
        /*[out,retval]*/ long * StartPoint ) = 0;
      virtual HRESULT __stdcall put_StartPoint (
        /*[in]*/ long StartPoint ) = 0;
      virtual HRESULT __stdcall get_EndPoint (
        /*[out,retval]*/ long * EndPoint ) = 0;
      virtual HRESULT __stdcall put_EndPoint (
        /*[in]*/ long EndPoint ) = 0;
      virtual HRESULT __stdcall get_FadeInDuration (
        /*[out,retval]*/ long * FadeInDuration ) = 0;
      virtual HRESULT __stdcall put_FadeInDuration (
        /*[in]*/ long FadeInDuration ) = 0;
      virtual HRESULT __stdcall get_FadeOutDuration (
        /*[out,retval]*/ long * FadeOutDuration ) = 0;
      virtual HRESULT __stdcall put_FadeOutDuration (
        /*[in]*/ long FadeOutDuration ) = 0;
      virtual HRESULT __stdcall get_MediaBookmarks (
        /*[out,retval]*/ struct MediaBookmarks * * MediaBookmarks ) = 0;
      virtual HRESULT __stdcall SetDisplayPicture (
        /*[in]*/ long Position ) = 0;
      virtual HRESULT __stdcall SetDisplayPictureFromFile (
        /*[in]*/ BSTR FilePath ) = 0;
      virtual HRESULT __stdcall Resample (
        /*[in]*/ VARIANT_BOOL Trim,
        /*[in]*/ int SampleHeight,
        /*[in]*/ int SampleWidth,
        /*[in]*/ long VideoFrameRate,
        /*[in]*/ long AudioSamplingRate,
        /*[in]*/ long VideoBitRate ) = 0;
      virtual HRESULT __stdcall ResampleFromProfile (
        /*[in]*/ enum PpResampleMediaProfile profile ) = 0;
      virtual HRESULT __stdcall get_ResamplingStatus (
        /*[out,retval]*/ enum PpMediaTaskStatus * ResamplingStatus ) = 0;
      virtual HRESULT __stdcall get_IsLinked (
        /*[out,retval]*/ VARIANT_BOOL * IsLinked ) = 0;
      virtual HRESULT __stdcall get_IsEmbedded (
        /*[out,retval]*/ VARIANT_BOOL * IsEmbedded ) = 0;
      virtual HRESULT __stdcall get_AudioSamplingRate (
        /*[out,retval]*/ long * AudioSamplingRate ) = 0;
      virtual HRESULT __stdcall get_VideoFrameRate (
        /*[out,retval]*/ long * VideoFrameRate ) = 0;
      virtual HRESULT __stdcall get_SampleHeight (
        /*[out,retval]*/ long * SampleHeight ) = 0;
      virtual HRESULT __stdcall get_SampleWidth (
        /*[out,retval]*/ long * SampleWidth ) = 0;
      virtual HRESULT __stdcall get_VideoCompressionType (
        /*[out,retval]*/ BSTR * VideoCompressionType ) = 0;
      virtual HRESULT __stdcall get_AudioCompressionType (
        /*[out,retval]*/ BSTR * AudioCompressionType ) = 0;
};

struct __declspec(uuid("ba72e557-4ff5-48f4-8215-5505f990966f"))
Coauthoring : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_MergeMode (
        /*[out,retval]*/ VARIANT_BOOL * MergeMode ) = 0;
      virtual HRESULT __stdcall get_FavorServerEditsDuringMerge (
        /*[out,retval]*/ VARIANT_BOOL * FavorServerEditsDuringMerge ) = 0;
      virtual HRESULT __stdcall put_FavorServerEditsDuringMerge (
        /*[in]*/ VARIANT_BOOL FavorServerEditsDuringMerge ) = 0;
      virtual HRESULT __stdcall EndReview ( ) = 0;
      virtual HRESULT __stdcall get_PendingUpdates (
        /*[out,retval]*/ VARIANT_BOOL * PendingUpdates ) = 0;
      virtual HRESULT __stdcall get_CoauthorCount (
        /*[out,retval]*/ long * CoauthorCount ) = 0;
};

struct __declspec(uuid("ba72e558-4ff5-48f4-8215-5505f990966f"))
Broadcast : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Start (
        /*[in]*/ BSTR serverUrl ) = 0;
      virtual HRESULT __stdcall End ( ) = 0;
      virtual HRESULT __stdcall get_AttendeeUrl (
        /*[out,retval]*/ BSTR * AttendeeUrl ) = 0;
      virtual HRESULT __stdcall get_IsBroadcasting (
        /*[out,retval]*/ VARIANT_BOOL * IsBroadcasting ) = 0;
};

enum __declspec(uuid("33e189f9-6f21-42c5-95cb-b064abf618ea"))
XlPieSliceLocation
{
    xlHorizontalCoordinate = 1,
    xlVerticalCoordinate = 2
};

enum __declspec(uuid("c11f48f8-2ecb-4a6d-81ce-96c4c604a765"))
XlPieSliceIndex
{
    xlOuterCounterClockwisePoint = 1,
    xlOuterCenterPoint = 2,
    xlOuterClockwisePoint = 3,
    xlMidClockwiseRadiusPoint = 4,
    xlCenterPoint = 5,
    xlMidCounterClockwiseRadiusPoint = 6,
    xlInnerClockwisePoint = 7,
    xlInnerCenterPoint = 8,
    xlInnerCounterClockwisePoint = 9
};

struct __declspec(uuid("92d41a73-f07e-4ca4-af6f-bef486aa4e6f"))
Point : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Border (
        /*[out,retval]*/ struct ChartBorder * * RHS ) = 0;
      virtual HRESULT __stdcall ClearFormats (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall Copy (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_DataLabel (
        /*[out,retval]*/ struct DataLabel * * RHS ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_Explosion (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_Explosion (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_HasDataLabel (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_HasDataLabel (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Interior (
        /*[out,retval]*/ struct Interior * * RHS ) = 0;
      virtual HRESULT __stdcall get_InvertIfNegative (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_InvertIfNegative (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_MarkerBackgroundColor (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_MarkerBackgroundColor (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_MarkerBackgroundColorIndex (
        /*[out,retval]*/ enum XlColorIndex * RHS ) = 0;
      virtual HRESULT __stdcall put_MarkerBackgroundColorIndex (
        /*[in]*/ enum XlColorIndex RHS ) = 0;
      virtual HRESULT __stdcall get_MarkerForegroundColor (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_MarkerForegroundColor (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_MarkerForegroundColorIndex (
        /*[out,retval]*/ enum XlColorIndex * RHS ) = 0;
      virtual HRESULT __stdcall put_MarkerForegroundColorIndex (
        /*[in]*/ enum XlColorIndex RHS ) = 0;
      virtual HRESULT __stdcall get_MarkerSize (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_MarkerSize (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_MarkerStyle (
        /*[out,retval]*/ enum XlMarkerStyle * RHS ) = 0;
      virtual HRESULT __stdcall put_MarkerStyle (
        /*[in]*/ enum XlMarkerStyle RHS ) = 0;
      virtual HRESULT __stdcall Paste (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_PictureType (
        /*[out,retval]*/ enum XlChartPictureType * RHS ) = 0;
      virtual HRESULT __stdcall put_PictureType (
        /*[in]*/ enum XlChartPictureType RHS ) = 0;
      virtual HRESULT __stdcall Select (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_ApplyPictToSides (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_ApplyPictToSides (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_ApplyPictToFront (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_ApplyPictToFront (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_ApplyPictToEnd (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_ApplyPictToEnd (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Shadow (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_Shadow (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_SecondaryPlot (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_SecondaryPlot (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Fill (
        /*[out,retval]*/ struct ChartFillFormat * * RHS ) = 0;
      virtual HRESULT __stdcall get_Has3DEffect (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_Has3DEffect (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_PictureUnit2 (
        /*[out,retval]*/ double * RHS ) = 0;
      virtual HRESULT __stdcall put_PictureUnit2 (
        /*[in]*/ double RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall _ApplyDataLabels (
        /*[in]*/ enum XlDataLabelsType Type,
        /*[in]*/ VARIANT LegendKey,
        /*[in]*/ VARIANT AutoText,
        /*[in]*/ VARIANT HasLeaderLines,
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall ApplyDataLabels (
        /*[in]*/ enum XlDataLabelsType Type,
        /*[in]*/ VARIANT LegendKey,
        /*[in]*/ VARIANT AutoText,
        /*[in]*/ VARIANT HasLeaderLines,
        /*[in]*/ VARIANT ShowSeriesName,
        /*[in]*/ VARIANT ShowCategoryName,
        /*[in]*/ VARIANT ShowValue,
        /*[in]*/ VARIANT ShowPercentage,
        /*[in]*/ VARIANT ShowBubbleSize,
        /*[in]*/ VARIANT Separator,
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall get_PictureUnit (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_PictureUnit (
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ double * pval ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ double * pval ) = 0;
      virtual HRESULT __stdcall get_Left (
        /*[out,retval]*/ double * pval ) = 0;
      virtual HRESULT __stdcall get_Top (
        /*[out,retval]*/ double * pval ) = 0;
      virtual HRESULT __stdcall PieSliceLocation (
        /*[in]*/ enum XlPieSliceLocation loc,
        /*[in]*/ enum XlPieSliceIndex Index,
        /*[out,retval]*/ double * pval ) = 0;
};

struct __declspec(uuid("92d41a74-f07e-4ca4-af6f-bef486aa4e6f"))
Points : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ long Index,
        /*[out,retval]*/ struct Point * * RHS ) = 0;
      virtual HRESULT __stdcall _NewEnum (
        /*[out,retval]*/ IUnknown * * RHS ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall _Default (
        /*[in]*/ long Index,
        /*[out,retval]*/ struct Point * * RHS ) = 0;
};

enum __declspec(uuid("7f9b36c7-48cc-335e-b058-49658fd8cece"))
PpProtectedViewCloseReason
{
    ppProtectedViewCloseNormal = 0,
    ppProtectedViewCloseEdit = 1,
    ppProtectedViewCloseForced = 2
};

struct __declspec(uuid("ba72e55a-4ff5-48f4-8215-5505f990966f"))
ProtectedViewWindow : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Presentation (
        /*[out,retval]*/ struct _Presentation * * Presentation ) = 0;
      virtual HRESULT __stdcall get_Active (
        /*[out,retval]*/ enum Office2010::MsoTriState * Active ) = 0;
      virtual HRESULT __stdcall get_WindowState (
        /*[out,retval]*/ enum PpWindowState * WindowState ) = 0;
      virtual HRESULT __stdcall put_WindowState (
        /*[in]*/ enum PpWindowState WindowState ) = 0;
      virtual HRESULT __stdcall get_Caption (
        /*[out,retval]*/ BSTR * Caption ) = 0;
      virtual HRESULT __stdcall get_SourcePath (
        /*[out,retval]*/ BSTR * SourcePath ) = 0;
      virtual HRESULT __stdcall get_SourceName (
        /*[out,retval]*/ BSTR * SourceName ) = 0;
      virtual HRESULT __stdcall get_Left (
        /*[out,retval]*/ float * Left ) = 0;
      virtual HRESULT __stdcall put_Left (
        /*[in]*/ float Left ) = 0;
      virtual HRESULT __stdcall get_Top (
        /*[out,retval]*/ float * Top ) = 0;
      virtual HRESULT __stdcall put_Top (
        /*[in]*/ float Top ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ float * Width ) = 0;
      virtual HRESULT __stdcall put_Width (
        /*[in]*/ float Width ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ float * Height ) = 0;
      virtual HRESULT __stdcall put_Height (
        /*[in]*/ float Height ) = 0;
      virtual HRESULT __stdcall Activate ( ) = 0;
      virtual HRESULT __stdcall Close ( ) = 0;
      virtual HRESULT __stdcall Edit (
        /*[in]*/ BSTR ModifyPassword,
        /*[out,retval]*/ struct _Presentation * * Edit ) = 0;
      virtual HRESULT __stdcall get_HWND (
        /*[out,retval]*/ long * HWND ) = 0;
};

struct __declspec(uuid("ba72e559-4ff5-48f4-8215-5505f990966f"))
ProtectedViewWindows : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct ProtectedViewWindow * * Item ) = 0;
      virtual HRESULT __stdcall Open (
        /*[in]*/ BSTR FileName,
        /*[in]*/ BSTR ReadPassword,
        /*[in]*/ enum Office2010::MsoTriState OpenAndRepair,
        /*[out,retval]*/ struct ProtectedViewWindow * * Open ) = 0;
};

struct __declspec(uuid("91493442-5a91-11cf-8700-00aa0060263b"))
_Application : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Presentations (
        /*[out,retval]*/ struct Presentations * * Presentations ) = 0;
      virtual HRESULT __stdcall get_Windows (
        /*[out,retval]*/ struct DocumentWindows * * Windows ) = 0;
      virtual HRESULT __stdcall get_Dialogs (
        /*[out,retval]*/ IUnknown * * Dialogs ) = 0;
      virtual HRESULT __stdcall get_ActiveWindow (
        /*[out,retval]*/ struct DocumentWindow * * ActiveWindow ) = 0;
      virtual HRESULT __stdcall get_ActivePresentation (
        /*[out,retval]*/ struct _Presentation * * ActivePresentation ) = 0;
      virtual HRESULT __stdcall get_SlideShowWindows (
        /*[out,retval]*/ struct SlideShowWindows * * SlideShowWindows ) = 0;
      virtual HRESULT __stdcall get_CommandBars (
        /*[out,retval]*/ struct Office2010::_CommandBars * * CommandBars ) = 0;
      virtual HRESULT __stdcall get_Path (
        /*[out,retval]*/ BSTR * Path ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall get_Caption (
        /*[out,retval]*/ BSTR * Caption ) = 0;
      virtual HRESULT __stdcall put_Caption (
        /*[in]*/ BSTR Caption ) = 0;
      virtual HRESULT __stdcall get_Assistant (
        /*[out,retval]*/ struct Office2010::Assistant * * Assistant ) = 0;
      virtual HRESULT __stdcall get_FileSearch (
        /*[out,retval]*/ struct Office2010::FileSearch * * FileSearch ) = 0;
      virtual HRESULT __stdcall get_FileFind (
        /*[out,retval]*/ struct Office2010::IFind * * FileFind ) = 0;
      virtual HRESULT __stdcall get_Build (
        /*[out,retval]*/ BSTR * Build ) = 0;
      virtual HRESULT __stdcall get_Version (
        /*[out,retval]*/ BSTR * Version ) = 0;
      virtual HRESULT __stdcall get_OperatingSystem (
        /*[out,retval]*/ BSTR * OperatingSystem ) = 0;
      virtual HRESULT __stdcall get_ActivePrinter (
        /*[out,retval]*/ BSTR * ActivePrinter ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * Creator ) = 0;
      virtual HRESULT __stdcall get_AddIns (
        /*[out,retval]*/ struct AddIns * * AddIns ) = 0;
      virtual HRESULT __stdcall get_VBE (
        /*[out,retval]*/ void * * VBE ) = 0;
      virtual HRESULT __stdcall Help (
        /*[in]*/ BSTR HelpFile,
        /*[in]*/ int ContextID ) = 0;
      virtual HRESULT __stdcall Quit ( ) = 0;
      virtual HRESULT __stdcall Run (
        /*[in]*/ BSTR MacroName,
        /*[in]*/ SAFEARRAY * * safeArrayOfParams,
        /*[out,retval]*/ VARIANT * Run ) = 0;
      virtual HRESULT __stdcall PPFileDialog (
        /*[in]*/ enum PpFileDialogType Type,
        /*[out,retval]*/ IUnknown * * PPFileDialog ) = 0;
      virtual HRESULT __stdcall LaunchSpelling (
        /*[in]*/ struct DocumentWindow * pWindow ) = 0;
      virtual HRESULT __stdcall get_Left (
        /*[out,retval]*/ float * Left ) = 0;
      virtual HRESULT __stdcall put_Left (
        /*[in]*/ float Left ) = 0;
      virtual HRESULT __stdcall get_Top (
        /*[out,retval]*/ float * Top ) = 0;
      virtual HRESULT __stdcall put_Top (
        /*[in]*/ float Top ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ float * Width ) = 0;
      virtual HRESULT __stdcall put_Width (
        /*[in]*/ float Width ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ float * Height ) = 0;
      virtual HRESULT __stdcall put_Height (
        /*[in]*/ float Height ) = 0;
      virtual HRESULT __stdcall get_WindowState (
        /*[out,retval]*/ enum PpWindowState * WindowState ) = 0;
      virtual HRESULT __stdcall put_WindowState (
        /*[in]*/ enum PpWindowState WindowState ) = 0;
      virtual HRESULT __stdcall get_Visible (
        /*[out,retval]*/ enum Office2010::MsoTriState * Visible ) = 0;
      virtual HRESULT __stdcall put_Visible (
        /*[in]*/ enum Office2010::MsoTriState Visible ) = 0;
      virtual HRESULT __stdcall get_HWND (
        /*[out,retval]*/ long * HWND ) = 0;
      virtual HRESULT __stdcall get_Active (
        /*[out,retval]*/ enum Office2010::MsoTriState * Active ) = 0;
      virtual HRESULT __stdcall Activate ( ) = 0;
      virtual HRESULT __stdcall get_AnswerWizard (
        /*[out,retval]*/ struct Office2010::AnswerWizard * * AnswerWizard ) = 0;
      virtual HRESULT __stdcall get_COMAddIns (
        /*[out,retval]*/ struct Office2010::COMAddIns * * COMAddIns ) = 0;
      virtual HRESULT __stdcall get_ProductCode (
        /*[out,retval]*/ BSTR * ProductCode ) = 0;
      virtual HRESULT __stdcall get_DefaultWebOptions (
        /*[out,retval]*/ struct DefaultWebOptions * * DefaultWebOptions ) = 0;
      virtual HRESULT __stdcall get_LanguageSettings (
        /*[out,retval]*/ struct Office2010::LanguageSettings * * LanguageSettings ) = 0;
      virtual HRESULT __stdcall get_MsoDebugOptions (
        /*[out,retval]*/ struct Office2010::MsoDebugOptions * * MsoDebugOptions ) = 0;
      virtual HRESULT __stdcall get_ShowWindowsInTaskbar (
        /*[out,retval]*/ enum Office2010::MsoTriState * ShowWindowsInTaskbar ) = 0;
      virtual HRESULT __stdcall put_ShowWindowsInTaskbar (
        /*[in]*/ enum Office2010::MsoTriState ShowWindowsInTaskbar ) = 0;
      virtual HRESULT __stdcall get_Marker (
        /*[out,retval]*/ IUnknown * * Marker ) = 0;
      virtual HRESULT __stdcall get_FeatureInstall (
        /*[out,retval]*/ enum Office2010::MsoFeatureInstall * FeatureInstall ) = 0;
      virtual HRESULT __stdcall put_FeatureInstall (
        /*[in]*/ enum Office2010::MsoFeatureInstall FeatureInstall ) = 0;
      virtual HRESULT __stdcall GetOptionFlag (
        /*[in]*/ long Option,
        /*[in]*/ VARIANT_BOOL Persist,
        /*[out,retval]*/ VARIANT_BOOL * GetOptionFlag ) = 0;
      virtual HRESULT __stdcall SetOptionFlag (
        /*[in]*/ long Option,
        /*[in]*/ VARIANT_BOOL State,
        /*[in]*/ VARIANT_BOOL Persist ) = 0;
      virtual HRESULT __stdcall get_FileDialog (
        /*[in]*/ enum Office2010::MsoFileDialogType Type,
        /*[out,retval]*/ struct Office2010::FileDialog * * FileDialog ) = 0;
      virtual HRESULT __stdcall get_DisplayGridLines (
        /*[out,retval]*/ enum Office2010::MsoTriState * DisplayGridLines ) = 0;
      virtual HRESULT __stdcall put_DisplayGridLines (
        /*[in]*/ enum Office2010::MsoTriState DisplayGridLines ) = 0;
      virtual HRESULT __stdcall get_AutomationSecurity (
        /*[out,retval]*/ enum Office2010::MsoAutomationSecurity * AutomationSecurity ) = 0;
      virtual HRESULT __stdcall put_AutomationSecurity (
        /*[in]*/ enum Office2010::MsoAutomationSecurity AutomationSecurity ) = 0;
      virtual HRESULT __stdcall get_NewPresentation (
        /*[out,retval]*/ struct Office2010::NewFile * * NewPresentation ) = 0;
      virtual HRESULT __stdcall get_DisplayAlerts (
        /*[out,retval]*/ enum PpAlertLevel * DisplayAlerts ) = 0;
      virtual HRESULT __stdcall put_DisplayAlerts (
        /*[in]*/ enum PpAlertLevel DisplayAlerts ) = 0;
      virtual HRESULT __stdcall get_ShowStartupDialog (
        /*[out,retval]*/ enum Office2010::MsoTriState * ShowStartupDialog ) = 0;
      virtual HRESULT __stdcall put_ShowStartupDialog (
        /*[in]*/ enum Office2010::MsoTriState ShowStartupDialog ) = 0;
      virtual HRESULT __stdcall SetPerfMarker (
        /*[in]*/ int Marker ) = 0;
      virtual HRESULT __stdcall get_AutoCorrect (
        /*[out,retval]*/ struct AutoCorrect * * AutoCorrect ) = 0;
      virtual HRESULT __stdcall get_Options (
        /*[out,retval]*/ struct Options * * Options ) = 0;
      virtual HRESULT __stdcall LaunchPublishSlidesDialog (
        /*[in]*/ BSTR SlideLibraryUrl ) = 0;
      virtual HRESULT __stdcall LaunchSendToPPTDialog (
        /*[in]*/ VARIANT * SlideUrls ) = 0;
      virtual HRESULT __stdcall get_DisplayDocumentInformationPanel (
        /*[out,retval]*/ VARIANT_BOOL * DisplayDocumentInformationPanel ) = 0;
      virtual HRESULT __stdcall put_DisplayDocumentInformationPanel (
        /*[in]*/ VARIANT_BOOL DisplayDocumentInformationPanel ) = 0;
      virtual HRESULT __stdcall get_Assistance (
        /*[out,retval]*/ struct Office2010::IAssistance * * Assistance ) = 0;
      virtual HRESULT __stdcall get_ActiveEncryptionSession (
        /*[out,retval]*/ long * ActiveEncryptionSession ) = 0;
      virtual HRESULT __stdcall get_FileConverters (
        /*[out,retval]*/ struct FileConverters * * FileConverters ) = 0;
      virtual HRESULT __stdcall get_SmartArtLayouts (
        /*[out,retval]*/ struct Office2010::SmartArtLayouts * * SmartArtLayouts ) = 0;
      virtual HRESULT __stdcall get_SmartArtQuickStyles (
        /*[out,retval]*/ struct Office2010::SmartArtQuickStyles * * SmartArtQuickStyles ) = 0;
      virtual HRESULT __stdcall get_SmartArtColors (
        /*[out,retval]*/ struct Office2010::SmartArtColors * * SmartArtColors ) = 0;
      virtual HRESULT __stdcall get_ProtectedViewWindows (
        /*[out,retval]*/ struct ProtectedViewWindows * * ProtectedViewWindows ) = 0;
      virtual HRESULT __stdcall get_ActiveProtectedViewWindow (
        /*[out,retval]*/ struct ProtectedViewWindow * * ActiveProtectedViewWindow ) = 0;
      virtual HRESULT __stdcall get_IsSandboxed (
        /*[out,retval]*/ VARIANT_BOOL * IsSandboxed ) = 0;
      virtual HRESULT __stdcall get_ResampleMediaTasks (
        /*[out,retval]*/ struct ResampleMediaTasks * * ResampleMediaTasks ) = 0;
      virtual HRESULT __stdcall StartNewUndoEntry ( ) = 0;
      virtual HRESULT __stdcall get_FileValidation (
        /*[out,retval]*/ enum Office2010::MsoFileValidationMode * FileValidation ) = 0;
      virtual HRESULT __stdcall put_FileValidation (
        /*[in]*/ enum Office2010::MsoFileValidationMode FileValidation ) = 0;
};

struct __declspec(uuid("91493451-5a91-11cf-8700-00aa0060263b"))
_Global : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_ActivePresentation (
        /*[out,retval]*/ struct _Presentation * * ActivePresentation ) = 0;
      virtual HRESULT __stdcall get_ActiveWindow (
        /*[out,retval]*/ struct DocumentWindow * * ActiveWindow ) = 0;
      virtual HRESULT __stdcall get_AddIns (
        /*[out,retval]*/ struct AddIns * * AddIns ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Assistant (
        /*[out,retval]*/ struct Office2010::Assistant * * Assistant ) = 0;
      virtual HRESULT __stdcall get_Dialogs (
        /*[out,retval]*/ IUnknown * * Dialogs ) = 0;
      virtual HRESULT __stdcall get_Presentations (
        /*[out,retval]*/ struct Presentations * * Presentations ) = 0;
      virtual HRESULT __stdcall get_SlideShowWindows (
        /*[out,retval]*/ struct SlideShowWindows * * SlideShowWindows ) = 0;
      virtual HRESULT __stdcall get_Windows (
        /*[out,retval]*/ struct DocumentWindows * * Windows ) = 0;
      virtual HRESULT __stdcall get_CommandBars (
        /*[out,retval]*/ struct Office2010::_CommandBars * * CommandBars ) = 0;
      virtual HRESULT __stdcall get_AnswerWizard (
        /*[out,retval]*/ struct Office2010::AnswerWizard * * AnswerWizard ) = 0;
      virtual HRESULT __stdcall get_FileConverters (
        /*[out,retval]*/ struct FileConverters * * FileConverters ) = 0;
      virtual HRESULT __stdcall get_ProtectedViewWindows (
        /*[out,retval]*/ struct ProtectedViewWindows * * ProtectedViewWindows ) = 0;
      virtual HRESULT __stdcall get_ActiveProtectedViewWindow (
        /*[out,retval]*/ struct ProtectedViewWindow * * ActiveProtectedViewWindow ) = 0;
      virtual HRESULT __stdcall get_IsSandboxed (
        /*[out,retval]*/ VARIANT_BOOL * IsSandboxed ) = 0;
};

struct __declspec(uuid("91493454-5a91-11cf-8700-00aa0060263b"))
Selection : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Cut ( ) = 0;
      virtual HRESULT __stdcall Copy ( ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
      virtual HRESULT __stdcall Unselect ( ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum PpSelectionType * Type ) = 0;
      virtual HRESULT __stdcall get_SlideRange (
        /*[out,retval]*/ struct SlideRange * * SlideRange ) = 0;
      virtual HRESULT __stdcall get_ShapeRange (
        /*[out,retval]*/ struct ShapeRange * * ShapeRange ) = 0;
      virtual HRESULT __stdcall get_TextRange (
        /*[out,retval]*/ struct TextRange * * TextRange ) = 0;
      virtual HRESULT __stdcall get_ChildShapeRange (
        /*[out,retval]*/ struct ShapeRange * * ChildShapeRange ) = 0;
      virtual HRESULT __stdcall get_HasChildShapeRange (
        /*[out,retval]*/ VARIANT_BOOL * HasChildShapeRange ) = 0;
      virtual HRESULT __stdcall get_TextRange2 (
        /*[out,retval]*/ struct Office2010::TextRange2 * * TextRange2 ) = 0;
};

struct __declspec(uuid("91493457-5a91-11cf-8700-00aa0060263b"))
DocumentWindow : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Selection (
        /*[out,retval]*/ struct Selection * * Selection ) = 0;
      virtual HRESULT __stdcall get_View (
        /*[out,retval]*/ struct View * * View ) = 0;
      virtual HRESULT __stdcall get_Presentation (
        /*[out,retval]*/ struct _Presentation * * Presentation ) = 0;
      virtual HRESULT __stdcall get_ViewType (
        /*[out,retval]*/ enum PpViewType * ViewType ) = 0;
      virtual HRESULT __stdcall put_ViewType (
        /*[in]*/ enum PpViewType ViewType ) = 0;
      virtual HRESULT __stdcall get_BlackAndWhite (
        /*[out,retval]*/ enum Office2010::MsoTriState * BlackAndWhite ) = 0;
      virtual HRESULT __stdcall put_BlackAndWhite (
        /*[in]*/ enum Office2010::MsoTriState BlackAndWhite ) = 0;
      virtual HRESULT __stdcall get_Active (
        /*[out,retval]*/ enum Office2010::MsoTriState * Active ) = 0;
      virtual HRESULT __stdcall get_WindowState (
        /*[out,retval]*/ enum PpWindowState * WindowState ) = 0;
      virtual HRESULT __stdcall put_WindowState (
        /*[in]*/ enum PpWindowState WindowState ) = 0;
      virtual HRESULT __stdcall get_Caption (
        /*[out,retval]*/ BSTR * Caption ) = 0;
      virtual HRESULT __stdcall get_Left (
        /*[out,retval]*/ float * Left ) = 0;
      virtual HRESULT __stdcall put_Left (
        /*[in]*/ float Left ) = 0;
      virtual HRESULT __stdcall get_Top (
        /*[out,retval]*/ float * Top ) = 0;
      virtual HRESULT __stdcall put_Top (
        /*[in]*/ float Top ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ float * Width ) = 0;
      virtual HRESULT __stdcall put_Width (
        /*[in]*/ float Width ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ float * Height ) = 0;
      virtual HRESULT __stdcall put_Height (
        /*[in]*/ float Height ) = 0;
      virtual HRESULT __stdcall FitToPage ( ) = 0;
      virtual HRESULT __stdcall Activate ( ) = 0;
      virtual HRESULT __stdcall LargeScroll (
        /*[in]*/ int Down,
        /*[in]*/ int Up,
        /*[in]*/ int ToRight,
        /*[in]*/ int ToLeft ) = 0;
      virtual HRESULT __stdcall SmallScroll (
        /*[in]*/ int Down,
        /*[in]*/ int Up,
        /*[in]*/ int ToRight,
        /*[in]*/ int ToLeft ) = 0;
      virtual HRESULT __stdcall NewWindow (
        /*[out,retval]*/ struct DocumentWindow * * NewWindow ) = 0;
      virtual HRESULT __stdcall Close ( ) = 0;
      virtual HRESULT __stdcall get_HWND (
        /*[out,retval]*/ long * HWND ) = 0;
      virtual HRESULT __stdcall get_ActivePane (
        /*[out,retval]*/ struct Pane * * ActivePane ) = 0;
      virtual HRESULT __stdcall get_Panes (
        /*[out,retval]*/ struct Panes * * Panes ) = 0;
      virtual HRESULT __stdcall get_SplitVertical (
        /*[out,retval]*/ long * SplitVertical ) = 0;
      virtual HRESULT __stdcall put_SplitVertical (
        /*[in]*/ long SplitVertical ) = 0;
      virtual HRESULT __stdcall get_SplitHorizontal (
        /*[out,retval]*/ long * SplitHorizontal ) = 0;
      virtual HRESULT __stdcall put_SplitHorizontal (
        /*[in]*/ long SplitHorizontal ) = 0;
      virtual HRESULT __stdcall RangeFromPoint (
        /*[in]*/ int X,
        /*[in]*/ int Y,
        /*[out,retval]*/ IDispatch * * RangeFromPoint ) = 0;
      virtual HRESULT __stdcall PointsToScreenPixelsX (
        /*[in]*/ float Points,
        /*[out,retval]*/ int * PointsToScreenPixelsX ) = 0;
      virtual HRESULT __stdcall PointsToScreenPixelsY (
        /*[in]*/ float Points,
        /*[out,retval]*/ int * PointsToScreenPixelsY ) = 0;
      virtual HRESULT __stdcall ScrollIntoView (
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[in]*/ enum Office2010::MsoTriState Start ) = 0;
      virtual HRESULT __stdcall IsSectionExpanded (
        /*[in]*/ int sectionIndex,
        /*[out,retval]*/ VARIANT_BOOL * IsSectionExpanded ) = 0;
      virtual HRESULT __stdcall ExpandSection (
        /*[in]*/ int sectionIndex,
        /*[in]*/ VARIANT_BOOL Expand ) = 0;
};

struct __declspec(uuid("91493455-5a91-11cf-8700-00aa0060263b"))
DocumentWindows : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct DocumentWindow * * Item ) = 0;
      virtual HRESULT __stdcall Arrange (
        /*[in]*/ enum PpArrangeStyle arrangeStyle ) = 0;
};

struct __declspec(uuid("91493469-5a91-11cf-8700-00aa0060263b"))
Slides : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct _Slide * * Item ) = 0;
      virtual HRESULT __stdcall FindBySlideID (
        /*[in]*/ long SlideID,
        /*[out,retval]*/ struct _Slide * * FindBySlideID ) = 0;
      virtual HRESULT __stdcall Add (
        /*[in]*/ int Index,
        /*[in]*/ enum PpSlideLayout Layout,
        /*[out,retval]*/ struct _Slide * * Add ) = 0;
      virtual HRESULT __stdcall InsertFromFile (
        /*[in]*/ BSTR FileName,
        /*[in]*/ int Index,
        /*[in]*/ int SlideStart,
        /*[in]*/ int SlideEnd,
        /*[out,retval]*/ int * FromFile ) = 0;
      virtual HRESULT __stdcall Range (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct SlideRange * * Range ) = 0;
      virtual HRESULT __stdcall Paste (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct SlideRange * * Paste ) = 0;
      virtual HRESULT __stdcall AddSlide (
        /*[in]*/ int Index,
        /*[in]*/ struct CustomLayout * pCustomLayout,
        /*[out,retval]*/ struct _Slide * * Slide ) = 0;
};

struct __declspec(uuid("9149346a-5a91-11cf-8700-00aa0060263b"))
_Slide : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Shapes (
        /*[out,retval]*/ struct Shapes * * Shapes ) = 0;
      virtual HRESULT __stdcall get_HeadersFooters (
        /*[out,retval]*/ struct HeadersFooters * * HeadersFooters ) = 0;
      virtual HRESULT __stdcall get_SlideShowTransition (
        /*[out,retval]*/ struct SlideShowTransition * * SlideShowTransition ) = 0;
      virtual HRESULT __stdcall get_ColorScheme (
        /*[out,retval]*/ struct ColorScheme * * ColorScheme ) = 0;
      virtual HRESULT __stdcall put_ColorScheme (
        /*[in]*/ struct ColorScheme * ColorScheme ) = 0;
      virtual HRESULT __stdcall get_Background (
        /*[out,retval]*/ struct ShapeRange * * Background ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall put_Name (
        /*[in]*/ BSTR Name ) = 0;
      virtual HRESULT __stdcall get_SlideID (
        /*[out,retval]*/ long * SlideID ) = 0;
      virtual HRESULT __stdcall get_PrintSteps (
        /*[out,retval]*/ int * PrintSteps ) = 0;
      virtual HRESULT __stdcall Select ( ) = 0;
      virtual HRESULT __stdcall Cut ( ) = 0;
      virtual HRESULT __stdcall Copy ( ) = 0;
      virtual HRESULT __stdcall get_Layout (
        /*[out,retval]*/ enum PpSlideLayout * Layout ) = 0;
      virtual HRESULT __stdcall put_Layout (
        /*[in]*/ enum PpSlideLayout Layout ) = 0;
      virtual HRESULT __stdcall Duplicate (
        /*[out,retval]*/ struct SlideRange * * Duplicate ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
      virtual HRESULT __stdcall get_Tags (
        /*[out,retval]*/ struct Tags * * Tags ) = 0;
      virtual HRESULT __stdcall get_SlideIndex (
        /*[out,retval]*/ int * SlideIndex ) = 0;
      virtual HRESULT __stdcall get_SlideNumber (
        /*[out,retval]*/ int * SlideNumber ) = 0;
      virtual HRESULT __stdcall get_DisplayMasterShapes (
        /*[out,retval]*/ enum Office2010::MsoTriState * DisplayMasterShapes ) = 0;
      virtual HRESULT __stdcall put_DisplayMasterShapes (
        /*[in]*/ enum Office2010::MsoTriState DisplayMasterShapes ) = 0;
      virtual HRESULT __stdcall get_FollowMasterBackground (
        /*[out,retval]*/ enum Office2010::MsoTriState * FollowMasterBackground ) = 0;
      virtual HRESULT __stdcall put_FollowMasterBackground (
        /*[in]*/ enum Office2010::MsoTriState FollowMasterBackground ) = 0;
      virtual HRESULT __stdcall get_NotesPage (
        /*[out,retval]*/ struct SlideRange * * NotesPage ) = 0;
      virtual HRESULT __stdcall get_Master (
        /*[out,retval]*/ struct _Master * * Master ) = 0;
      virtual HRESULT __stdcall get_Hyperlinks (
        /*[out,retval]*/ struct Hyperlinks * * Hyperlinks ) = 0;
      virtual HRESULT __stdcall Export (
        /*[in]*/ BSTR FileName,
        /*[in]*/ BSTR FilterName,
        /*[in]*/ int ScaleWidth,
        /*[in]*/ int ScaleHeight ) = 0;
      virtual HRESULT __stdcall get_Scripts (
        /*[out,retval]*/ struct Office2010::Scripts * * Scripts ) = 0;
      virtual HRESULT __stdcall get_Comments (
        /*[out,retval]*/ struct Comments * * Comments ) = 0;
      virtual HRESULT __stdcall get_Design (
        /*[out,retval]*/ struct Design * * Design ) = 0;
      virtual HRESULT __stdcall put_Design (
        /*[in]*/ struct Design * Design ) = 0;
      virtual HRESULT __stdcall MoveTo (
        /*[in]*/ int toPos ) = 0;
      virtual HRESULT __stdcall get_TimeLine (
        /*[out,retval]*/ struct TimeLine * * TimeLine ) = 0;
      virtual HRESULT __stdcall ApplyTemplate (
        /*[in]*/ BSTR FileName ) = 0;
      virtual HRESULT __stdcall get_SectionNumber (
        /*[out,retval]*/ int * SectionNumber ) = 0;
      virtual HRESULT __stdcall get_CustomLayout (
        /*[out,retval]*/ struct CustomLayout * * CustomLayout ) = 0;
      virtual HRESULT __stdcall put_CustomLayout (
        /*[in]*/ struct CustomLayout * CustomLayout ) = 0;
      virtual HRESULT __stdcall ApplyTheme (
        /*[in]*/ BSTR themeName ) = 0;
      virtual HRESULT __stdcall get_ThemeColorScheme (
        /*[out,retval]*/ struct Office2010::ThemeColorScheme * * ThemeColorScheme ) = 0;
      virtual HRESULT __stdcall ApplyThemeColorScheme (
        /*[in]*/ BSTR themeColorSchemeName ) = 0;
      virtual HRESULT __stdcall get_BackgroundStyle (
        /*[out,retval]*/ enum Office2010::MsoBackgroundStyleIndex * BackgroundStyle ) = 0;
      virtual HRESULT __stdcall put_BackgroundStyle (
        /*[in]*/ enum Office2010::MsoBackgroundStyleIndex BackgroundStyle ) = 0;
      virtual HRESULT __stdcall get_CustomerData (
        /*[out,retval]*/ struct CustomerData * * CustomerData ) = 0;
      virtual HRESULT __stdcall PublishSlides (
        /*[in]*/ BSTR SlideLibraryUrl,
        /*[in]*/ VARIANT_BOOL Overwrite,
        /*[in]*/ VARIANT_BOOL UseSlideOrder ) = 0;
      virtual HRESULT __stdcall MoveToSectionStart (
        /*[in]*/ int toSection ) = 0;
      virtual HRESULT __stdcall get_sectionIndex (
        /*[out,retval]*/ int * sectionIndex ) = 0;
      virtual HRESULT __stdcall get_HasNotesPage (
        /*[out,retval]*/ enum Office2010::MsoTriState * HasNotesPage ) = 0;
};

struct __declspec(uuid("9149346b-5a91-11cf-8700-00aa0060263b"))
SlideRange : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Shapes (
        /*[out,retval]*/ struct Shapes * * Shapes ) = 0;
      virtual HRESULT __stdcall get_HeadersFooters (
        /*[out,retval]*/ struct HeadersFooters * * HeadersFooters ) = 0;
      virtual HRESULT __stdcall get_SlideShowTransition (
        /*[out,retval]*/ struct SlideShowTransition * * SlideShowTransition ) = 0;
      virtual HRESULT __stdcall get_ColorScheme (
        /*[out,retval]*/ struct ColorScheme * * ColorScheme ) = 0;
      virtual HRESULT __stdcall put_ColorScheme (
        /*[in]*/ struct ColorScheme * ColorScheme ) = 0;
      virtual HRESULT __stdcall get_Background (
        /*[out,retval]*/ struct ShapeRange * * Background ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall put_Name (
        /*[in]*/ BSTR Name ) = 0;
      virtual HRESULT __stdcall get_SlideID (
        /*[out,retval]*/ long * SlideID ) = 0;
      virtual HRESULT __stdcall get_PrintSteps (
        /*[out,retval]*/ int * PrintSteps ) = 0;
      virtual HRESULT __stdcall Select ( ) = 0;
      virtual HRESULT __stdcall Cut ( ) = 0;
      virtual HRESULT __stdcall Copy ( ) = 0;
      virtual HRESULT __stdcall get_Layout (
        /*[out,retval]*/ enum PpSlideLayout * Layout ) = 0;
      virtual HRESULT __stdcall put_Layout (
        /*[in]*/ enum PpSlideLayout Layout ) = 0;
      virtual HRESULT __stdcall Duplicate (
        /*[out,retval]*/ struct SlideRange * * Duplicate ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
      virtual HRESULT __stdcall get_Tags (
        /*[out,retval]*/ struct Tags * * Tags ) = 0;
      virtual HRESULT __stdcall get_SlideIndex (
        /*[out,retval]*/ int * SlideIndex ) = 0;
      virtual HRESULT __stdcall get_SlideNumber (
        /*[out,retval]*/ int * SlideNumber ) = 0;
      virtual HRESULT __stdcall get_DisplayMasterShapes (
        /*[out,retval]*/ enum Office2010::MsoTriState * DisplayMasterShapes ) = 0;
      virtual HRESULT __stdcall put_DisplayMasterShapes (
        /*[in]*/ enum Office2010::MsoTriState DisplayMasterShapes ) = 0;
      virtual HRESULT __stdcall get_FollowMasterBackground (
        /*[out,retval]*/ enum Office2010::MsoTriState * FollowMasterBackground ) = 0;
      virtual HRESULT __stdcall put_FollowMasterBackground (
        /*[in]*/ enum Office2010::MsoTriState FollowMasterBackground ) = 0;
      virtual HRESULT __stdcall get_NotesPage (
        /*[out,retval]*/ struct SlideRange * * NotesPage ) = 0;
      virtual HRESULT __stdcall get_Master (
        /*[out,retval]*/ struct _Master * * Master ) = 0;
      virtual HRESULT __stdcall get_Hyperlinks (
        /*[out,retval]*/ struct Hyperlinks * * Hyperlinks ) = 0;
      virtual HRESULT __stdcall Export (
        /*[in]*/ BSTR FileName,
        /*[in]*/ BSTR FilterName,
        /*[in]*/ int ScaleWidth,
        /*[in]*/ int ScaleHeight ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct _Slide * * Item ) = 0;
      virtual HRESULT __stdcall get__NewEnum (
        /*[out,retval]*/ IUnknown * * _NewEnum ) = 0;
      virtual HRESULT __stdcall _Index (
        /*[in]*/ int Index,
        /*[out,retval]*/ VARIANT * _Index ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ long * Count ) = 0;
      virtual HRESULT __stdcall get_Scripts (
        /*[out,retval]*/ struct Office2010::Scripts * * Scripts ) = 0;
      virtual HRESULT __stdcall get_Comments (
        /*[out,retval]*/ struct Comments * * Comments ) = 0;
      virtual HRESULT __stdcall get_Design (
        /*[out,retval]*/ struct Design * * Design ) = 0;
      virtual HRESULT __stdcall put_Design (
        /*[in]*/ struct Design * Design ) = 0;
      virtual HRESULT __stdcall MoveTo (
        /*[in]*/ int toPos ) = 0;
      virtual HRESULT __stdcall get_TimeLine (
        /*[out,retval]*/ struct TimeLine * * TimeLine ) = 0;
      virtual HRESULT __stdcall ApplyTemplate (
        /*[in]*/ BSTR FileName ) = 0;
      virtual HRESULT __stdcall get_SectionNumber (
        /*[out,retval]*/ int * SectionNumber ) = 0;
      virtual HRESULT __stdcall get_CustomLayout (
        /*[out,retval]*/ struct CustomLayout * * CustomLayout ) = 0;
      virtual HRESULT __stdcall put_CustomLayout (
        /*[in]*/ struct CustomLayout * CustomLayout ) = 0;
      virtual HRESULT __stdcall ApplyTheme (
        /*[in]*/ BSTR themeName ) = 0;
      virtual HRESULT __stdcall get_ThemeColorScheme (
        /*[out,retval]*/ struct Office2010::ThemeColorScheme * * ThemeColorScheme ) = 0;
      virtual HRESULT __stdcall ApplyThemeColorScheme (
        /*[in]*/ BSTR themeColorSchemeName ) = 0;
      virtual HRESULT __stdcall get_BackgroundStyle (
        /*[out,retval]*/ enum Office2010::MsoBackgroundStyleIndex * BackgroundStyle ) = 0;
      virtual HRESULT __stdcall put_BackgroundStyle (
        /*[in]*/ enum Office2010::MsoBackgroundStyleIndex BackgroundStyle ) = 0;
      virtual HRESULT __stdcall get_CustomerData (
        /*[out,retval]*/ struct CustomerData * * CustomerData ) = 0;
      virtual HRESULT __stdcall PublishSlides (
        /*[in]*/ BSTR SlideLibraryUrl,
        /*[in]*/ VARIANT_BOOL Overwrite,
        /*[in]*/ VARIANT_BOOL UseSlideOrder ) = 0;
      virtual HRESULT __stdcall MoveToSectionStart (
        /*[in]*/ int toSection ) = 0;
      virtual HRESULT __stdcall get_sectionIndex (
        /*[out,retval]*/ int * sectionIndex ) = 0;
      virtual HRESULT __stdcall get_HasNotesPage (
        /*[out,retval]*/ enum Office2010::MsoTriState * HasNotesPage ) = 0;
};

struct __declspec(uuid("9149346c-5a91-11cf-8700-00aa0060263b"))
_Master : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Shapes (
        /*[out,retval]*/ struct Shapes * * Shapes ) = 0;
      virtual HRESULT __stdcall get_HeadersFooters (
        /*[out,retval]*/ struct HeadersFooters * * HeadersFooters ) = 0;
      virtual HRESULT __stdcall get_ColorScheme (
        /*[out,retval]*/ struct ColorScheme * * ColorScheme ) = 0;
      virtual HRESULT __stdcall put_ColorScheme (
        /*[in]*/ struct ColorScheme * ColorScheme ) = 0;
      virtual HRESULT __stdcall get_Background (
        /*[out,retval]*/ struct ShapeRange * * Background ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall put_Name (
        /*[in]*/ BSTR Name ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ float * Height ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ float * Width ) = 0;
      virtual HRESULT __stdcall get_TextStyles (
        /*[out,retval]*/ struct TextStyles * * TextStyles ) = 0;
      virtual HRESULT __stdcall get_Hyperlinks (
        /*[out,retval]*/ struct Hyperlinks * * Hyperlinks ) = 0;
      virtual HRESULT __stdcall get_Scripts (
        /*[out,retval]*/ struct Office2010::Scripts * * Scripts ) = 0;
      virtual HRESULT __stdcall get_Design (
        /*[out,retval]*/ struct Design * * Design ) = 0;
      virtual HRESULT __stdcall get_TimeLine (
        /*[out,retval]*/ struct TimeLine * * TimeLine ) = 0;
      virtual HRESULT __stdcall get_SlideShowTransition (
        /*[out,retval]*/ struct SlideShowTransition * * SlideShowTransition ) = 0;
      virtual HRESULT __stdcall get_CustomLayouts (
        /*[out,retval]*/ struct CustomLayouts * * CustomLayouts ) = 0;
      virtual HRESULT __stdcall get_Theme (
        /*[out,retval]*/ struct Office2010::OfficeTheme * * Theme ) = 0;
      virtual HRESULT __stdcall ApplyTheme (
        /*[in]*/ BSTR themeName ) = 0;
      virtual HRESULT __stdcall get_BackgroundStyle (
        /*[out,retval]*/ enum Office2010::MsoBackgroundStyleIndex * BackgroundStyle ) = 0;
      virtual HRESULT __stdcall put_BackgroundStyle (
        /*[in]*/ enum Office2010::MsoBackgroundStyleIndex BackgroundStyle ) = 0;
      virtual HRESULT __stdcall get_CustomerData (
        /*[out,retval]*/ struct CustomerData * * CustomerData ) = 0;
};

struct __declspec(uuid("914934d7-5a91-11cf-8700-00aa0060263b"))
Design : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_SlideMaster (
        /*[out,retval]*/ struct _Master * * SlideMaster ) = 0;
      virtual HRESULT __stdcall get_TitleMaster (
        /*[out,retval]*/ struct _Master * * TitleMaster ) = 0;
      virtual HRESULT __stdcall get_HasTitleMaster (
        /*[out,retval]*/ enum Office2010::MsoTriState * HasTitleMaster ) = 0;
      virtual HRESULT __stdcall AddTitleMaster (
        /*[out,retval]*/ struct _Master * * TitleMaster ) = 0;
      virtual HRESULT __stdcall get_Index (
        /*[out,retval]*/ int * Index ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall put_Name (
        /*[in]*/ BSTR Name ) = 0;
      virtual HRESULT __stdcall get_Preserved (
        /*[out,retval]*/ enum Office2010::MsoTriState * Preserved ) = 0;
      virtual HRESULT __stdcall put_Preserved (
        /*[in]*/ enum Office2010::MsoTriState Preserved ) = 0;
      virtual HRESULT __stdcall MoveTo (
        /*[in]*/ int toPos ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
};

struct __declspec(uuid("914934d6-5a91-11cf-8700-00aa0060263b"))
Designs : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct Design * * Item ) = 0;
      virtual HRESULT __stdcall Add (
        /*[in]*/ BSTR designName,
        /*[in]*/ int Index,
        /*[out,retval]*/ struct Design * * Add ) = 0;
      virtual HRESULT __stdcall Load (
        /*[in]*/ BSTR TemplateName,
        /*[in]*/ int Index,
        /*[out,retval]*/ struct Design * * Load ) = 0;
      virtual HRESULT __stdcall Clone (
        /*[in]*/ struct Design * pOriginal,
        /*[in]*/ int Index,
        /*[out,retval]*/ struct Design * * Clone ) = 0;
};

struct __declspec(uuid("91493475-5a91-11cf-8700-00aa0060263b"))
Shapes : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ int * Count ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct Shape * * Item ) = 0;
      virtual HRESULT __stdcall get__NewEnum (
        /*[out,retval]*/ IUnknown * * _NewEnum ) = 0;
      virtual HRESULT __stdcall AddCallout (
        /*[in]*/ enum Office2010::MsoCalloutType Type,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * Callout ) = 0;
      virtual HRESULT __stdcall AddConnector (
        /*[in]*/ enum Office2010::MsoConnectorType Type,
        /*[in]*/ float BeginX,
        /*[in]*/ float BeginY,
        /*[in]*/ float EndX,
        /*[in]*/ float EndY,
        /*[out,retval]*/ struct Shape * * Connector ) = 0;
      virtual HRESULT __stdcall AddCurve (
        /*[in]*/ VARIANT SafeArrayOfPoints,
        /*[out,retval]*/ struct Shape * * Curve ) = 0;
      virtual HRESULT __stdcall AddLabel (
        /*[in]*/ enum Office2010::MsoTextOrientation Orientation,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * Label ) = 0;
      virtual HRESULT __stdcall AddLine (
        /*[in]*/ float BeginX,
        /*[in]*/ float BeginY,
        /*[in]*/ float EndX,
        /*[in]*/ float EndY,
        /*[out,retval]*/ struct Shape * * Line ) = 0;
      virtual HRESULT __stdcall AddPicture (
        /*[in]*/ BSTR FileName,
        /*[in]*/ enum Office2010::MsoTriState LinkToFile,
        /*[in]*/ enum Office2010::MsoTriState SaveWithDocument,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * Picture ) = 0;
      virtual HRESULT __stdcall AddPolyline (
        /*[in]*/ VARIANT SafeArrayOfPoints,
        /*[out,retval]*/ struct Shape * * Polyline ) = 0;
      virtual HRESULT __stdcall AddShape (
        /*[in]*/ enum Office2010::MsoAutoShapeType Type,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * Shape ) = 0;
      virtual HRESULT __stdcall AddTextEffect (
        /*[in]*/ enum Office2010::MsoPresetTextEffect PresetTextEffect,
        /*[in]*/ BSTR Text,
        /*[in]*/ BSTR FontName,
        /*[in]*/ float FontSize,
        /*[in]*/ enum Office2010::MsoTriState FontBold,
        /*[in]*/ enum Office2010::MsoTriState FontItalic,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[out,retval]*/ struct Shape * * TextEffect ) = 0;
      virtual HRESULT __stdcall AddTextbox (
        /*[in]*/ enum Office2010::MsoTextOrientation Orientation,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * Textbox ) = 0;
      virtual HRESULT __stdcall BuildFreeform (
        /*[in]*/ enum Office2010::MsoEditingType EditingType,
        /*[in]*/ float X1,
        /*[in]*/ float Y1,
        /*[out,retval]*/ struct FreeformBuilder * * FreeformBuilder ) = 0;
      virtual HRESULT __stdcall SelectAll ( ) = 0;
      virtual HRESULT __stdcall Range (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct ShapeRange * * Range ) = 0;
      virtual HRESULT __stdcall get_HasTitle (
        /*[out,retval]*/ enum Office2010::MsoTriState * HasTitle ) = 0;
      virtual HRESULT __stdcall AddTitle (
        /*[out,retval]*/ struct Shape * * Title ) = 0;
      virtual HRESULT __stdcall get_Title (
        /*[out,retval]*/ struct Shape * * Title ) = 0;
      virtual HRESULT __stdcall get_Placeholders (
        /*[out,retval]*/ struct Placeholders * * Placeholders ) = 0;
      virtual HRESULT __stdcall AddOLEObject (
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[in]*/ BSTR ClassName,
        /*[in]*/ BSTR FileName,
        /*[in]*/ enum Office2010::MsoTriState DisplayAsIcon,
        /*[in]*/ BSTR IconFileName,
        /*[in]*/ int IconIndex,
        /*[in]*/ BSTR IconLabel,
        /*[in]*/ enum Office2010::MsoTriState Link,
        /*[out,retval]*/ struct Shape * * OLEObject ) = 0;
      virtual HRESULT __stdcall AddComment (
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * Comment ) = 0;
      virtual HRESULT __stdcall AddPlaceholder (
        /*[in]*/ enum PpPlaceholderType Type,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * Placeholder ) = 0;
      virtual HRESULT __stdcall AddMediaObject (
        /*[in]*/ BSTR FileName,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * MediaObject ) = 0;
      virtual HRESULT __stdcall Paste (
        /*[out,retval]*/ struct ShapeRange * * Paste ) = 0;
      virtual HRESULT __stdcall AddTable (
        /*[in]*/ int NumRows,
        /*[in]*/ int NumColumns,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * Table ) = 0;
      virtual HRESULT __stdcall PasteSpecial (
        /*[in]*/ enum PpPasteDataType DataType,
        /*[in]*/ enum Office2010::MsoTriState DisplayAsIcon,
        /*[in]*/ BSTR IconFileName,
        /*[in]*/ int IconIndex,
        /*[in]*/ BSTR IconLabel,
        /*[in]*/ enum Office2010::MsoTriState Link,
        /*[out,retval]*/ struct ShapeRange * * PasteSpecial ) = 0;
      virtual HRESULT __stdcall AddDiagram (
        /*[in]*/ enum Office2010::MsoDiagramType Type,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * Diagram ) = 0;
      virtual HRESULT __stdcall AddCanvas (
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * Shape ) = 0;
      virtual HRESULT __stdcall AddChart (
        /*[in]*/ enum Office2010::XlChartType Type,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * Chart ) = 0;
      virtual HRESULT __stdcall AddMediaObject2 (
        /*[in]*/ BSTR FileName,
        /*[in]*/ enum Office2010::MsoTriState LinkToFile,
        /*[in]*/ enum Office2010::MsoTriState SaveWithDocument,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * MediaObject2 ) = 0;
      virtual HRESULT __stdcall AddMediaObjectFromEmbedTag (
        /*[in]*/ BSTR EmbedTag,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * MediaObjectFromEmbedTag ) = 0;
      virtual HRESULT __stdcall AddSmartArt (
        /*[in]*/ struct Office2010::SmartArtLayout * Layout,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * SmartArt ) = 0;
};

struct __declspec(uuid("92d41a55-f07e-4ca4-af6f-bef486aa4e6f"))
Chart : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall ApplyDataLabels (
        /*[in]*/ enum XlDataLabelsType Type,
        /*[in]*/ VARIANT LegendKey = vtMissing,
        /*[in]*/ VARIANT AutoText = vtMissing,
        /*[in]*/ VARIANT HasLeaderLines = vtMissing,
        /*[in]*/ VARIANT ShowSeriesName = vtMissing,
        /*[in]*/ VARIANT ShowCategoryName = vtMissing,
        /*[in]*/ VARIANT ShowValue = vtMissing,
        /*[in]*/ VARIANT ShowPercentage = vtMissing,
        /*[in]*/ VARIANT ShowBubbleSize = vtMissing,
        /*[in]*/ VARIANT Separator = vtMissing ) = 0;
      virtual HRESULT __stdcall get_ChartType (
        /*[out,retval]*/ enum Office2010::XlChartType * RHS ) = 0;
      virtual HRESULT __stdcall put_ChartType (
        /*[in]*/ enum Office2010::XlChartType RHS ) = 0;
      virtual HRESULT __stdcall get_HasDataTable (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_HasDataTable (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall ApplyCustomType (
        /*[in]*/ enum Office2010::XlChartType ChartType,
        /*[in]*/ VARIANT TypeName = vtMissing ) = 0;
      virtual HRESULT __stdcall GetChartElement (
        /*[in]*/ long X,
        /*[in]*/ long Y,
        /*[in,out]*/ long * ElementID,
        /*[in,out]*/ long * Arg1,
        /*[in,out]*/ long * Arg2 ) = 0;
      virtual HRESULT __stdcall SetSourceData (
        /*[in]*/ BSTR Source,
        /*[in]*/ VARIANT PlotBy = vtMissing ) = 0;
      virtual HRESULT __stdcall get_PlotBy (
        /*[out,retval]*/ enum XlRowCol * PlotBy ) = 0;
      virtual HRESULT __stdcall put_PlotBy (
        /*[in]*/ enum XlRowCol PlotBy ) = 0;
      virtual HRESULT __stdcall AutoFormat (
        /*[in]*/ long Gallery,
        /*[in]*/ VARIANT Format = vtMissing ) = 0;
      virtual HRESULT __stdcall SetBackgroundPicture (
        /*[in]*/ BSTR FileName ) = 0;
      virtual HRESULT __stdcall get_DataTable (
        /*[out,retval]*/ struct DataTable * * RHS ) = 0;
      virtual HRESULT __stdcall Paste (
        /*[in]*/ VARIANT Type = vtMissing,
        /*[in,lcid]*/ long lcid = 0 ) = 0;
      virtual HRESULT __stdcall get_BarShape (
        /*[out,retval]*/ enum XlBarShape * RHS ) = 0;
      virtual HRESULT __stdcall put_BarShape (
        /*[in]*/ enum XlBarShape RHS ) = 0;
      virtual HRESULT __stdcall SetDefaultChart (
        /*[in]*/ VARIANT Name ) = 0;
      virtual HRESULT __stdcall ApplyChartTemplate (
        /*[in]*/ BSTR FileName ) = 0;
      virtual HRESULT __stdcall SaveChartTemplate (
        /*[in]*/ BSTR FileName ) = 0;
      virtual HRESULT __stdcall get_SideWall (
        /*[out,retval]*/ struct Walls * * RHS ) = 0;
      virtual HRESULT __stdcall get_BackWall (
        /*[out,retval]*/ struct Walls * * RHS ) = 0;
      virtual HRESULT __stdcall get_ChartStyle (
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_ChartStyle (
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall ClearToMatchStyle ( ) = 0;
      virtual HRESULT __stdcall get_HasPivotFields (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_HasPivotFields (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall put_ShowDataLabelsOverMaximum (
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_ShowDataLabelsOverMaximum (
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall ApplyLayout (
        /*[in]*/ long Layout,
        /*[in]*/ VARIANT ChartType = vtMissing ) = 0;
      virtual HRESULT __stdcall Refresh ( ) = 0;
      virtual HRESULT __stdcall get_ChartData (
        /*[out,retval]*/ struct ChartData * * ppchartdata ) = 0;
      virtual HRESULT __stdcall get_Shapes (
        /*[out,retval]*/ struct Shapes * * RHS ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * pval ) = 0;
      virtual HRESULT __stdcall get_Area3DGroup (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ struct ChartGroup * * RHS ) = 0;
      virtual HRESULT __stdcall AreaGroups (
        /*[in]*/ VARIANT Index,
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Bar3DGroup (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ struct ChartGroup * * RHS ) = 0;
      virtual HRESULT __stdcall BarGroups (
        /*[in]*/ VARIANT Index,
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Column3DGroup (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ struct ChartGroup * * RHS ) = 0;
      virtual HRESULT __stdcall ColumnGroups (
        /*[in]*/ VARIANT Index,
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Line3DGroup (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ struct ChartGroup * * RHS ) = 0;
      virtual HRESULT __stdcall LineGroups (
        /*[in]*/ VARIANT Index,
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Pie3DGroup (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ struct ChartGroup * * RHS ) = 0;
      virtual HRESULT __stdcall PieGroups (
        /*[in]*/ VARIANT Index,
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall DoughnutGroups (
        /*[in]*/ VARIANT Index,
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall RadarGroups (
        /*[in]*/ VARIANT Index,
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_SurfaceGroup (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ struct ChartGroup * * RHS ) = 0;
      virtual HRESULT __stdcall XYGroups (
        /*[in]*/ VARIANT Index,
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall _ApplyDataLabels (
        /*[in]*/ enum XlDataLabelsType Type,
        /*[in]*/ VARIANT LegendKey = vtMissing,
        /*[in]*/ VARIANT AutoText = vtMissing,
        /*[in]*/ VARIANT HasLeaderLines = vtMissing,
        /*[in,lcid]*/ long lcid = 0 ) = 0;
      virtual HRESULT __stdcall put_AutoScaling (
        /*[in,lcid]*/ long lcid,
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_AutoScaling (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall Axes (
        /*[in]*/ VARIANT Type,
        /*[in]*/ enum XlAxisGroup AxisGroup,
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_ChartArea (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ struct ChartArea * * ChartArea ) = 0;
      virtual HRESULT __stdcall ChartGroups (
        /*[in]*/ VARIANT Index,
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall get_ChartTitle (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ struct ChartTitle * * RHS ) = 0;
      virtual HRESULT __stdcall ChartWizard (
        /*[in]*/ VARIANT Source = vtMissing,
        /*[in]*/ VARIANT Gallery = vtMissing,
        /*[in]*/ VARIANT Format = vtMissing,
        /*[in]*/ VARIANT PlotBy = vtMissing,
        /*[in]*/ VARIANT CategoryLabels = vtMissing,
        /*[in]*/ VARIANT SeriesLabels = vtMissing,
        /*[in]*/ VARIANT HasLegend = vtMissing,
        /*[in]*/ VARIANT Title = vtMissing,
        /*[in]*/ VARIANT CategoryTitle = vtMissing,
        /*[in]*/ VARIANT ValueTitle = vtMissing,
        /*[in]*/ VARIANT ExtraTitle = vtMissing,
        /*[in,lcid]*/ long lcid = 0 ) = 0;
      virtual HRESULT __stdcall Copy (
        /*[in]*/ VARIANT Before = vtMissing,
        /*[in]*/ VARIANT After = vtMissing,
        /*[in,lcid]*/ long lcid = 0 ) = 0;
      virtual HRESULT __stdcall CopyPicture (
        /*[in]*/ enum XlPictureAppearance Appearance,
        /*[in]*/ enum XlCopyPictureFormat Format,
        /*[in]*/ enum XlPictureAppearance Size,
        /*[in,lcid]*/ long LocaleID = 0 ) = 0;
      virtual HRESULT __stdcall get_Corners (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ struct Corners * * RHS ) = 0;
      virtual HRESULT __stdcall Delete (
        /*[in,lcid]*/ long lcid = 0 ) = 0;
      virtual HRESULT __stdcall get_DepthPercent (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_DepthPercent (
        /*[in,lcid]*/ long lcid,
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_DisplayBlanksAs (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ enum XlDisplayBlanksAs * RHS ) = 0;
      virtual HRESULT __stdcall put_DisplayBlanksAs (
        /*[in,lcid]*/ long lcid,
        /*[in]*/ enum XlDisplayBlanksAs RHS ) = 0;
      virtual HRESULT __stdcall get_Elevation (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_Elevation (
        /*[in,lcid]*/ long lcid,
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall Export (
        /*[in]*/ BSTR FileName,
        /*[in]*/ VARIANT FilterName,
        /*[in]*/ VARIANT Interactive,
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall get_Floor (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ struct Floor * * RHS ) = 0;
      virtual HRESULT __stdcall get_GapDepth (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_GapDepth (
        /*[in,lcid]*/ long lcid,
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_HasAxis (
        /*[in]*/ VARIANT Index1,
        /*[in]*/ VARIANT Index2,
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_HasAxis (
        /*[in]*/ VARIANT Index1,
        /*[in]*/ VARIANT Index2 = vtMissing,
        /*[in,lcid]*/ long lcid = 0,
        /*[in]*/ VARIANT RHS = vtMissing ) = 0;
      virtual HRESULT __stdcall get_HasLegend (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_HasLegend (
        /*[in,lcid]*/ long lcid,
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_HasTitle (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_HasTitle (
        /*[in,lcid]*/ long lcid,
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_HeightPercent (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_HeightPercent (
        /*[in,lcid]*/ long lcid,
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_Legend (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ struct Legend * * RHS ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * RHS ) = 0;
      virtual HRESULT __stdcall put_Name (
        /*[in]*/ BSTR RHS ) = 0;
      virtual HRESULT __stdcall get_Perspective (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_Perspective (
        /*[in,lcid]*/ long lcid,
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_PlotArea (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ struct PlotArea * * RHS ) = 0;
      virtual HRESULT __stdcall get_PlotVisibleOnly (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ VARIANT_BOOL * RHS ) = 0;
      virtual HRESULT __stdcall put_PlotVisibleOnly (
        /*[in,lcid]*/ long lcid,
        /*[in]*/ VARIANT_BOOL RHS ) = 0;
      virtual HRESULT __stdcall get_RightAngleAxes (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_RightAngleAxes (
        /*[in,lcid]*/ long lcid,
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall get_Rotation (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ VARIANT * RHS ) = 0;
      virtual HRESULT __stdcall put_Rotation (
        /*[in,lcid]*/ long lcid,
        /*[in]*/ VARIANT RHS ) = 0;
      virtual HRESULT __stdcall Select (
        /*[in]*/ VARIANT Replace = vtMissing,
        /*[in,lcid]*/ long lcid = 0 ) = 0;
      virtual HRESULT __stdcall SeriesCollection (
        /*[in]*/ VARIANT Index,
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ IDispatch * * RHS ) = 0;
      virtual HRESULT __stdcall SetElement (
        /*[in]*/ enum Office2010::MsoChartElementType Element ) = 0;
      virtual HRESULT __stdcall get_Subtype (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_Subtype (
        /*[in,lcid]*/ long lcid,
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ long * RHS ) = 0;
      virtual HRESULT __stdcall put_Type (
        /*[in,lcid]*/ long lcid,
        /*[in]*/ long RHS ) = 0;
      virtual HRESULT __stdcall get_Walls (
        /*[in,lcid]*/ long lcid,
        /*[out,retval]*/ struct Walls * * RHS ) = 0;
      virtual HRESULT __stdcall get_Format (
        /*[out,retval]*/ struct ChartFormat * * ppChartFormat ) = 0;
      virtual HRESULT __stdcall get_ShowReportFilterFieldButtons (
        /*[out,retval]*/ VARIANT_BOOL * res ) = 0;
      virtual HRESULT __stdcall put_ShowReportFilterFieldButtons (
        /*[in]*/ VARIANT_BOOL res ) = 0;
      virtual HRESULT __stdcall get_ShowLegendFieldButtons (
        /*[out,retval]*/ VARIANT_BOOL * res ) = 0;
      virtual HRESULT __stdcall put_ShowLegendFieldButtons (
        /*[in]*/ VARIANT_BOOL res ) = 0;
      virtual HRESULT __stdcall get_ShowAxisFieldButtons (
        /*[out,retval]*/ VARIANT_BOOL * res ) = 0;
      virtual HRESULT __stdcall put_ShowAxisFieldButtons (
        /*[in]*/ VARIANT_BOOL res ) = 0;
      virtual HRESULT __stdcall get_ShowValueFieldButtons (
        /*[out,retval]*/ VARIANT_BOOL * res ) = 0;
      virtual HRESULT __stdcall put_ShowValueFieldButtons (
        /*[in]*/ VARIANT_BOOL res ) = 0;
      virtual HRESULT __stdcall get_ShowAllFieldButtons (
        /*[out,retval]*/ VARIANT_BOOL * res ) = 0;
      virtual HRESULT __stdcall put_ShowAllFieldButtons (
        /*[in]*/ VARIANT_BOOL res ) = 0;
      virtual HRESULT __stdcall get_AlternativeText (
        /*[out,retval]*/ BSTR * AlternativeText ) = 0;
      virtual HRESULT __stdcall put_AlternativeText (
        /*[in]*/ BSTR AlternativeText ) = 0;
      virtual HRESULT __stdcall get_Title (
        /*[out,retval]*/ BSTR * Title ) = 0;
      virtual HRESULT __stdcall put_Title (
        /*[in]*/ BSTR Title ) = 0;
};

struct __declspec(uuid("91493476-5a91-11cf-8700-00aa0060263b"))
Placeholders : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct Shape * * Item ) = 0;
      virtual HRESULT __stdcall FindByName (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct Shape * * FindByName ) = 0;
};

struct __declspec(uuid("91493478-5a91-11cf-8700-00aa0060263b"))
FreeformBuilder : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall AddNodes (
        /*[in]*/ enum Office2010::MsoSegmentType SegmentType,
        /*[in]*/ enum Office2010::MsoEditingType EditingType,
        /*[in]*/ float X1,
        /*[in]*/ float Y1,
        /*[in]*/ float X2,
        /*[in]*/ float Y2,
        /*[in]*/ float X3,
        /*[in]*/ float Y3 ) = 0;
      virtual HRESULT __stdcall ConvertToShape (
        /*[out,retval]*/ struct Shape * * Freeform ) = 0;
};

struct __declspec(uuid("91493479-5a91-11cf-8700-00aa0060263b"))
Shape : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Apply ( ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
      virtual HRESULT __stdcall Flip (
        /*[in]*/ enum Office2010::MsoFlipCmd FlipCmd ) = 0;
      virtual HRESULT __stdcall IncrementLeft (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall IncrementRotation (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall IncrementTop (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall PickUp ( ) = 0;
      virtual HRESULT __stdcall RerouteConnections ( ) = 0;
      virtual HRESULT __stdcall ScaleHeight (
        /*[in]*/ float Factor,
        /*[in]*/ enum Office2010::MsoTriState RelativeToOriginalSize,
        /*[in]*/ enum Office2010::MsoScaleFrom fScale ) = 0;
      virtual HRESULT __stdcall ScaleWidth (
        /*[in]*/ float Factor,
        /*[in]*/ enum Office2010::MsoTriState RelativeToOriginalSize,
        /*[in]*/ enum Office2010::MsoScaleFrom fScale ) = 0;
      virtual HRESULT __stdcall SetShapesDefaultProperties ( ) = 0;
      virtual HRESULT __stdcall Ungroup (
        /*[out,retval]*/ struct ShapeRange * * Ungroup ) = 0;
      virtual HRESULT __stdcall ZOrder (
        /*[in]*/ enum Office2010::MsoZOrderCmd ZOrderCmd ) = 0;
      virtual HRESULT __stdcall get_Adjustments (
        /*[out,retval]*/ struct Adjustments * * Adjustments ) = 0;
      virtual HRESULT __stdcall get_AutoShapeType (
        /*[out,retval]*/ enum Office2010::MsoAutoShapeType * AutoShapeType ) = 0;
      virtual HRESULT __stdcall put_AutoShapeType (
        /*[in]*/ enum Office2010::MsoAutoShapeType AutoShapeType ) = 0;
      virtual HRESULT __stdcall get_BlackWhiteMode (
        /*[out,retval]*/ enum Office2010::MsoBlackWhiteMode * BlackWhiteMode ) = 0;
      virtual HRESULT __stdcall put_BlackWhiteMode (
        /*[in]*/ enum Office2010::MsoBlackWhiteMode BlackWhiteMode ) = 0;
      virtual HRESULT __stdcall get_Callout (
        /*[out,retval]*/ struct CalloutFormat * * Callout ) = 0;
      virtual HRESULT __stdcall get_ConnectionSiteCount (
        /*[out,retval]*/ int * ConnectionSiteCount ) = 0;
      virtual HRESULT __stdcall get_Connector (
        /*[out,retval]*/ enum Office2010::MsoTriState * Connector ) = 0;
      virtual HRESULT __stdcall get_ConnectorFormat (
        /*[out,retval]*/ struct ConnectorFormat * * ConnectorFormat ) = 0;
      virtual HRESULT __stdcall get_Fill (
        /*[out,retval]*/ struct FillFormat * * Fill ) = 0;
      virtual HRESULT __stdcall get_GroupItems (
        /*[out,retval]*/ struct GroupShapes * * GroupItems ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ float * Height ) = 0;
      virtual HRESULT __stdcall put_Height (
        /*[in]*/ float Height ) = 0;
      virtual HRESULT __stdcall get_HorizontalFlip (
        /*[out,retval]*/ enum Office2010::MsoTriState * HorizontalFlip ) = 0;
      virtual HRESULT __stdcall get_Left (
        /*[out,retval]*/ float * Left ) = 0;
      virtual HRESULT __stdcall put_Left (
        /*[in]*/ float Left ) = 0;
      virtual HRESULT __stdcall get_Line (
        /*[out,retval]*/ struct LineFormat * * Line ) = 0;
      virtual HRESULT __stdcall get_LockAspectRatio (
        /*[out,retval]*/ enum Office2010::MsoTriState * LockAspectRatio ) = 0;
      virtual HRESULT __stdcall put_LockAspectRatio (
        /*[in]*/ enum Office2010::MsoTriState LockAspectRatio ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall put_Name (
        /*[in]*/ BSTR Name ) = 0;
      virtual HRESULT __stdcall get_Nodes (
        /*[out,retval]*/ struct ShapeNodes * * Nodes ) = 0;
      virtual HRESULT __stdcall get_Rotation (
        /*[out,retval]*/ float * Rotation ) = 0;
      virtual HRESULT __stdcall put_Rotation (
        /*[in]*/ float Rotation ) = 0;
      virtual HRESULT __stdcall get_PictureFormat (
        /*[out,retval]*/ struct PictureFormat * * Picture ) = 0;
      virtual HRESULT __stdcall get_Shadow (
        /*[out,retval]*/ struct ShadowFormat * * Shadow ) = 0;
      virtual HRESULT __stdcall get_TextEffect (
        /*[out,retval]*/ struct TextEffectFormat * * TextEffect ) = 0;
      virtual HRESULT __stdcall get_TextFrame (
        /*[out,retval]*/ struct TextFrame * * TextFrame ) = 0;
      virtual HRESULT __stdcall get_ThreeD (
        /*[out,retval]*/ struct ThreeDFormat * * ThreeD ) = 0;
      virtual HRESULT __stdcall get_Top (
        /*[out,retval]*/ float * Top ) = 0;
      virtual HRESULT __stdcall put_Top (
        /*[in]*/ float Top ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum Office2010::MsoShapeType * Type ) = 0;
      virtual HRESULT __stdcall get_VerticalFlip (
        /*[out,retval]*/ enum Office2010::MsoTriState * VerticalFlip ) = 0;
      virtual HRESULT __stdcall get_Vertices (
        /*[out,retval]*/ VARIANT * Vertices ) = 0;
      virtual HRESULT __stdcall get_Visible (
        /*[out,retval]*/ enum Office2010::MsoTriState * Visible ) = 0;
      virtual HRESULT __stdcall put_Visible (
        /*[in]*/ enum Office2010::MsoTriState Visible ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ float * Width ) = 0;
      virtual HRESULT __stdcall put_Width (
        /*[in]*/ float Width ) = 0;
      virtual HRESULT __stdcall get_ZOrderPosition (
        /*[out,retval]*/ int * ZOrderPosition ) = 0;
      virtual HRESULT __stdcall get_OLEFormat (
        /*[out,retval]*/ struct OLEFormat * * OLEFormat ) = 0;
      virtual HRESULT __stdcall get_LinkFormat (
        /*[out,retval]*/ struct LinkFormat * * LinkFormat ) = 0;
      virtual HRESULT __stdcall get_PlaceholderFormat (
        /*[out,retval]*/ struct PlaceholderFormat * * PlaceholderFormat ) = 0;
      virtual HRESULT __stdcall get_AnimationSettings (
        /*[out,retval]*/ struct AnimationSettings * * AnimationSettings ) = 0;
      virtual HRESULT __stdcall get_ActionSettings (
        /*[out,retval]*/ struct ActionSettings * * ActionSettings ) = 0;
      virtual HRESULT __stdcall get_Tags (
        /*[out,retval]*/ struct Tags * * Tags ) = 0;
      virtual HRESULT __stdcall Cut ( ) = 0;
      virtual HRESULT __stdcall Copy ( ) = 0;
      virtual HRESULT __stdcall Select (
        /*[in]*/ enum Office2010::MsoTriState Replace ) = 0;
      virtual HRESULT __stdcall Duplicate (
        /*[out,retval]*/ struct ShapeRange * * Duplicate ) = 0;
      virtual HRESULT __stdcall get_MediaType (
        /*[out,retval]*/ enum PpMediaType * MediaType ) = 0;
      virtual HRESULT __stdcall get_HasTextFrame (
        /*[out,retval]*/ enum Office2010::MsoTriState * HasTextFrame ) = 0;
      virtual HRESULT __stdcall get_SoundFormat (
        /*[out,retval]*/ struct SoundFormat * * SoundFormat ) = 0;
      virtual HRESULT __stdcall get_Script (
        /*[out,retval]*/ struct Office2010::Script * * Script ) = 0;
      virtual HRESULT __stdcall get_AlternativeText (
        /*[out,retval]*/ BSTR * AlternativeText ) = 0;
      virtual HRESULT __stdcall put_AlternativeText (
        /*[in]*/ BSTR AlternativeText ) = 0;
      virtual HRESULT __stdcall get_HasTable (
        /*[out,retval]*/ enum Office2010::MsoTriState * HasTable ) = 0;
      virtual HRESULT __stdcall get_Table (
        /*[out,retval]*/ struct Table * * Table ) = 0;
      virtual HRESULT __stdcall Export (
        /*[in]*/ BSTR PathName,
        /*[in]*/ enum PpShapeFormat Filter,
        /*[in]*/ int ScaleWidth,
        /*[in]*/ int ScaleHeight,
        /*[in]*/ enum PpExportMode ExportMode ) = 0;
      virtual HRESULT __stdcall get_HasDiagram (
        /*[out,retval]*/ enum Office2010::MsoTriState * pHasDiagram ) = 0;
      virtual HRESULT __stdcall get_Diagram (
        /*[out,retval]*/ struct Diagram * * Diagram ) = 0;
      virtual HRESULT __stdcall get_HasDiagramNode (
        /*[out,retval]*/ enum Office2010::MsoTriState * pHasDiagram ) = 0;
      virtual HRESULT __stdcall get_DiagramNode (
        /*[out,retval]*/ struct DiagramNode * * DiagramNode ) = 0;
      virtual HRESULT __stdcall get_Child (
        /*[out,retval]*/ enum Office2010::MsoTriState * Child ) = 0;
      virtual HRESULT __stdcall get_ParentGroup (
        /*[out,retval]*/ struct Shape * * Parent ) = 0;
      virtual HRESULT __stdcall get_CanvasItems (
        /*[out,retval]*/ struct CanvasShapes * * CanvasShapes ) = 0;
      virtual HRESULT __stdcall get_Id (
        /*[out,retval]*/ int * pid ) = 0;
      virtual HRESULT __stdcall CanvasCropLeft (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall CanvasCropTop (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall CanvasCropRight (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall CanvasCropBottom (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall put_RTF (
        /*[in]*/ BSTR _arg1 ) = 0;
      virtual HRESULT __stdcall get_CustomerData (
        /*[out,retval]*/ struct CustomerData * * CustomerData ) = 0;
      virtual HRESULT __stdcall get_TextFrame2 (
        /*[out,retval]*/ struct TextFrame2 * * Frame ) = 0;
      virtual HRESULT __stdcall get_HasChart (
        /*[out,retval]*/ enum Office2010::MsoTriState * pHasChart ) = 0;
      virtual HRESULT __stdcall get_ShapeStyle (
        /*[out,retval]*/ enum Office2010::MsoShapeStyleIndex * ShapeStyle ) = 0;
      virtual HRESULT __stdcall put_ShapeStyle (
        /*[in]*/ enum Office2010::MsoShapeStyleIndex ShapeStyle ) = 0;
      virtual HRESULT __stdcall get_BackgroundStyle (
        /*[out,retval]*/ enum Office2010::MsoBackgroundStyleIndex * BackgroundStyle ) = 0;
      virtual HRESULT __stdcall put_BackgroundStyle (
        /*[in]*/ enum Office2010::MsoBackgroundStyleIndex BackgroundStyle ) = 0;
      virtual HRESULT __stdcall get_SoftEdge (
        /*[out,retval]*/ struct Office2010::SoftEdgeFormat * * SoftEdge ) = 0;
      virtual HRESULT __stdcall get_Glow (
        /*[out,retval]*/ struct Office2010::GlowFormat * * Glow ) = 0;
      virtual HRESULT __stdcall get_Reflection (
        /*[out,retval]*/ struct Office2010::ReflectionFormat * * Reflection ) = 0;
      virtual HRESULT __stdcall get_Chart (
        /*[out,retval]*/ struct Chart * * Chart ) = 0;
      virtual HRESULT __stdcall get_HasSmartArt (
        /*[out,retval]*/ enum Office2010::MsoTriState * HasSmartArt ) = 0;
      virtual HRESULT __stdcall get_SmartArt (
        /*[out,retval]*/ struct Office2010::SmartArt * * SmartArt ) = 0;
      virtual HRESULT __stdcall ConvertTextToSmartArt (
        /*[in]*/ struct Office2010::SmartArtLayout * Layout ) = 0;
      virtual HRESULT __stdcall get_Title (
        /*[out,retval]*/ BSTR * Title ) = 0;
      virtual HRESULT __stdcall put_Title (
        /*[in]*/ BSTR Title ) = 0;
      virtual HRESULT __stdcall get_MediaFormat (
        /*[out,retval]*/ struct MediaFormat * * MediaFormat ) = 0;
      virtual HRESULT __stdcall PickupAnimation ( ) = 0;
      virtual HRESULT __stdcall ApplyAnimation ( ) = 0;
      virtual HRESULT __stdcall UpgradeMedia ( ) = 0;
};

struct __declspec(uuid("91493481-5a91-11cf-8700-00aa0060263b"))
ConnectorFormat : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall BeginConnect (
        /*[in]*/ struct Shape * ConnectedShape,
        /*[in]*/ int ConnectionSite ) = 0;
      virtual HRESULT __stdcall BeginDisconnect ( ) = 0;
      virtual HRESULT __stdcall EndConnect (
        /*[in]*/ struct Shape * ConnectedShape,
        /*[in]*/ int ConnectionSite ) = 0;
      virtual HRESULT __stdcall EndDisconnect ( ) = 0;
      virtual HRESULT __stdcall get_BeginConnected (
        /*[out,retval]*/ enum Office2010::MsoTriState * BeginConnected ) = 0;
      virtual HRESULT __stdcall get_BeginConnectedShape (
        /*[out,retval]*/ struct Shape * * BeginConnectedShape ) = 0;
      virtual HRESULT __stdcall get_BeginConnectionSite (
        /*[out,retval]*/ int * BeginConnectionSite ) = 0;
      virtual HRESULT __stdcall get_EndConnected (
        /*[out,retval]*/ enum Office2010::MsoTriState * EndConnected ) = 0;
      virtual HRESULT __stdcall get_EndConnectedShape (
        /*[out,retval]*/ struct Shape * * EndConnectedShape ) = 0;
      virtual HRESULT __stdcall get_EndConnectionSite (
        /*[out,retval]*/ int * EndConnectionSite ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum Office2010::MsoConnectorType * Type ) = 0;
      virtual HRESULT __stdcall put_Type (
        /*[in]*/ enum Office2010::MsoConnectorType Type ) = 0;
};

struct __declspec(uuid("9149349d-5a91-11cf-8700-00aa0060263b"))
_Presentation : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_SlideMaster (
        /*[out,retval]*/ struct _Master * * SlideMaster ) = 0;
      virtual HRESULT __stdcall get_TitleMaster (
        /*[out,retval]*/ struct _Master * * TitleMaster ) = 0;
      virtual HRESULT __stdcall get_HasTitleMaster (
        /*[out,retval]*/ enum Office2010::MsoTriState * HasTitleMaster ) = 0;
      virtual HRESULT __stdcall AddTitleMaster (
        /*[out,retval]*/ struct _Master * * TitleMaster ) = 0;
      virtual HRESULT __stdcall ApplyTemplate (
        /*[in]*/ BSTR FileName ) = 0;
      virtual HRESULT __stdcall get_TemplateName (
        /*[out,retval]*/ BSTR * TemplateName ) = 0;
      virtual HRESULT __stdcall get_NotesMaster (
        /*[out,retval]*/ struct _Master * * NotesMaster ) = 0;
      virtual HRESULT __stdcall get_HandoutMaster (
        /*[out,retval]*/ struct _Master * * HandoutMaster ) = 0;
      virtual HRESULT __stdcall get_Slides (
        /*[out,retval]*/ struct Slides * * Slides ) = 0;
      virtual HRESULT __stdcall get_PageSetup (
        /*[out,retval]*/ struct PageSetup * * PageSetup ) = 0;
      virtual HRESULT __stdcall get_ColorSchemes (
        /*[out,retval]*/ struct ColorSchemes * * ColorSchemes ) = 0;
      virtual HRESULT __stdcall get_ExtraColors (
        /*[out,retval]*/ struct ExtraColors * * ExtraColors ) = 0;
      virtual HRESULT __stdcall get_SlideShowSettings (
        /*[out,retval]*/ struct SlideShowSettings * * SlideShowSettings ) = 0;
      virtual HRESULT __stdcall get_Fonts (
        /*[out,retval]*/ struct Fonts * * Fonts ) = 0;
      virtual HRESULT __stdcall get_Windows (
        /*[out,retval]*/ struct DocumentWindows * * Windows ) = 0;
      virtual HRESULT __stdcall get_Tags (
        /*[out,retval]*/ struct Tags * * Tags ) = 0;
      virtual HRESULT __stdcall get_DefaultShape (
        /*[out,retval]*/ struct Shape * * DefaultShape ) = 0;
      virtual HRESULT __stdcall get_BuiltInDocumentProperties (
        /*[out,retval]*/ IDispatch * * BuiltInDocumentProperties ) = 0;
      virtual HRESULT __stdcall get_CustomDocumentProperties (
        /*[out,retval]*/ IDispatch * * CustomDocumentProperties ) = 0;
      virtual HRESULT __stdcall get_VBProject (
        /*[out,retval]*/ void * * VBProject ) = 0;
      virtual HRESULT __stdcall get_ReadOnly (
        /*[out,retval]*/ enum Office2010::MsoTriState * ReadOnly ) = 0;
      virtual HRESULT __stdcall get_FullName (
        /*[out,retval]*/ BSTR * FullName ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall get_Path (
        /*[out,retval]*/ BSTR * Path ) = 0;
      virtual HRESULT __stdcall get_Saved (
        /*[out,retval]*/ enum Office2010::MsoTriState * Saved ) = 0;
      virtual HRESULT __stdcall put_Saved (
        /*[in]*/ enum Office2010::MsoTriState Saved ) = 0;
      virtual HRESULT __stdcall get_LayoutDirection (
        /*[out,retval]*/ enum PpDirection * LayoutDirection ) = 0;
      virtual HRESULT __stdcall put_LayoutDirection (
        /*[in]*/ enum PpDirection LayoutDirection ) = 0;
      virtual HRESULT __stdcall NewWindow (
        /*[out,retval]*/ struct DocumentWindow * * NewWindow ) = 0;
      virtual HRESULT __stdcall FollowHyperlink (
        /*[in]*/ BSTR Address,
        /*[in]*/ BSTR SubAddress,
        /*[in]*/ VARIANT_BOOL NewWindow,
        /*[in]*/ VARIANT_BOOL AddHistory,
        /*[in]*/ BSTR ExtraInfo,
        /*[in]*/ enum Office2010::MsoExtraInfoMethod Method,
        /*[in]*/ BSTR HeaderInfo ) = 0;
      virtual HRESULT __stdcall AddToFavorites ( ) = 0;
      virtual HRESULT __stdcall Unused ( ) = 0;
      virtual HRESULT __stdcall get_PrintOptions (
        /*[out,retval]*/ struct PrintOptions * * PrintOptions ) = 0;
      virtual HRESULT __stdcall PrintOut (
        /*[in]*/ int From,
        /*[in]*/ int To,
        /*[in]*/ BSTR PrintToFile,
        /*[in]*/ int Copies,
        /*[in]*/ enum Office2010::MsoTriState Collate ) = 0;
      virtual HRESULT __stdcall Save ( ) = 0;
      virtual HRESULT __stdcall SaveAs (
        /*[in]*/ BSTR FileName,
        /*[in]*/ enum PpSaveAsFileType FileFormat,
        /*[in]*/ enum Office2010::MsoTriState EmbedTrueTypeFonts ) = 0;
      virtual HRESULT __stdcall SaveCopyAs (
        /*[in]*/ BSTR FileName,
        /*[in]*/ enum PpSaveAsFileType FileFormat,
        /*[in]*/ enum Office2010::MsoTriState EmbedTrueTypeFonts ) = 0;
      virtual HRESULT __stdcall Export (
        /*[in]*/ BSTR Path,
        /*[in]*/ BSTR FilterName,
        /*[in]*/ int ScaleWidth,
        /*[in]*/ int ScaleHeight ) = 0;
      virtual HRESULT __stdcall Close ( ) = 0;
      virtual HRESULT __stdcall SetUndoText (
        /*[in]*/ BSTR Text ) = 0;
      virtual HRESULT __stdcall get_Container (
        /*[out,retval]*/ IDispatch * * Container ) = 0;
      virtual HRESULT __stdcall get_DisplayComments (
        /*[out,retval]*/ enum Office2010::MsoTriState * DisplayComments ) = 0;
      virtual HRESULT __stdcall put_DisplayComments (
        /*[in]*/ enum Office2010::MsoTriState DisplayComments ) = 0;
      virtual HRESULT __stdcall get_FarEastLineBreakLevel (
        /*[out,retval]*/ enum PpFarEastLineBreakLevel * FarEastLineBreakLevel ) = 0;
      virtual HRESULT __stdcall put_FarEastLineBreakLevel (
        /*[in]*/ enum PpFarEastLineBreakLevel FarEastLineBreakLevel ) = 0;
      virtual HRESULT __stdcall get_NoLineBreakBefore (
        /*[out,retval]*/ BSTR * NoLineBreakBefore ) = 0;
      virtual HRESULT __stdcall put_NoLineBreakBefore (
        /*[in]*/ BSTR NoLineBreakBefore ) = 0;
      virtual HRESULT __stdcall get_NoLineBreakAfter (
        /*[out,retval]*/ BSTR * NoLineBreakAfter ) = 0;
      virtual HRESULT __stdcall put_NoLineBreakAfter (
        /*[in]*/ BSTR NoLineBreakAfter ) = 0;
      virtual HRESULT __stdcall UpdateLinks ( ) = 0;
      virtual HRESULT __stdcall get_SlideShowWindow (
        /*[out,retval]*/ struct SlideShowWindow * * SlideShowWindow ) = 0;
      virtual HRESULT __stdcall get_FarEastLineBreakLanguage (
        /*[out,retval]*/ enum Office2010::MsoFarEastLineBreakLanguageID * FarEastLineBreakLanguage ) = 0;
      virtual HRESULT __stdcall put_FarEastLineBreakLanguage (
        /*[in]*/ enum Office2010::MsoFarEastLineBreakLanguageID FarEastLineBreakLanguage ) = 0;
      virtual HRESULT __stdcall WebPagePreview ( ) = 0;
      virtual HRESULT __stdcall get_DefaultLanguageID (
        /*[out,retval]*/ enum Office2010::MsoLanguageID * DefaultLanguageID ) = 0;
      virtual HRESULT __stdcall put_DefaultLanguageID (
        /*[in]*/ enum Office2010::MsoLanguageID DefaultLanguageID ) = 0;
      virtual HRESULT __stdcall get_CommandBars (
        /*[out,retval]*/ struct Office2010::_CommandBars * * CommandBars ) = 0;
      virtual HRESULT __stdcall get_PublishObjects (
        /*[out,retval]*/ struct PublishObjects * * PublishObjects ) = 0;
      virtual HRESULT __stdcall get_WebOptions (
        /*[out,retval]*/ struct WebOptions * * WebOptions ) = 0;
      virtual HRESULT __stdcall get_HTMLProject (
        /*[out,retval]*/ struct Office2010::HTMLProject * * HTMLProject ) = 0;
      virtual HRESULT __stdcall ReloadAs (
        /*[in]*/ enum Office2010::MsoEncoding cp ) = 0;
      virtual HRESULT __stdcall MakeIntoTemplate (
        /*[in]*/ enum Office2010::MsoTriState IsDesignTemplate ) = 0;
      virtual HRESULT __stdcall get_EnvelopeVisible (
        /*[out,retval]*/ enum Office2010::MsoTriState * EnvelopeVisible ) = 0;
      virtual HRESULT __stdcall put_EnvelopeVisible (
        /*[in]*/ enum Office2010::MsoTriState EnvelopeVisible ) = 0;
      virtual HRESULT __stdcall sblt (
        /*[in]*/ BSTR s ) = 0;
      virtual HRESULT __stdcall get_VBASigned (
        /*[out,retval]*/ enum Office2010::MsoTriState * VBASigned ) = 0;
      virtual HRESULT __stdcall get_SnapToGrid (
        /*[out,retval]*/ enum Office2010::MsoTriState * SnapToGrid ) = 0;
      virtual HRESULT __stdcall put_SnapToGrid (
        /*[in]*/ enum Office2010::MsoTriState SnapToGrid ) = 0;
      virtual HRESULT __stdcall get_GridDistance (
        /*[out,retval]*/ float * GridDistance ) = 0;
      virtual HRESULT __stdcall put_GridDistance (
        /*[in]*/ float GridDistance ) = 0;
      virtual HRESULT __stdcall get_Designs (
        /*[out,retval]*/ struct Designs * * Designs ) = 0;
      virtual HRESULT __stdcall Merge (
        /*[in]*/ BSTR Path ) = 0;
      virtual HRESULT __stdcall CheckIn (
        /*[in]*/ VARIANT_BOOL SaveChanges,
        /*[in]*/ VARIANT Comments = vtMissing,
        /*[in]*/ VARIANT MakePublic = vtMissing ) = 0;
      virtual HRESULT __stdcall CanCheckIn (
        /*[out,retval]*/ VARIANT_BOOL * CanCheckIn ) = 0;
      virtual HRESULT __stdcall get_Signatures (
        /*[out,retval]*/ struct Office2010::SignatureSet * * Signatures ) = 0;
      virtual HRESULT __stdcall get_RemovePersonalInformation (
        /*[out,retval]*/ enum Office2010::MsoTriState * RemovePersonalInformation ) = 0;
      virtual HRESULT __stdcall put_RemovePersonalInformation (
        /*[in]*/ enum Office2010::MsoTriState RemovePersonalInformation ) = 0;
      virtual HRESULT __stdcall SendForReview (
        /*[in]*/ BSTR Recipients,
        /*[in]*/ BSTR Subject,
        /*[in]*/ VARIANT_BOOL ShowMessage,
        /*[in]*/ VARIANT IncludeAttachment = vtMissing ) = 0;
      virtual HRESULT __stdcall ReplyWithChanges (
        /*[in]*/ VARIANT_BOOL ShowMessage ) = 0;
      virtual HRESULT __stdcall EndReview ( ) = 0;
      virtual HRESULT __stdcall get_HasRevisionInfo (
        /*[out,retval]*/ enum PpRevisionInfo * HasRevisionInfo ) = 0;
      virtual HRESULT __stdcall AddBaseline (
        /*[in]*/ BSTR FileName ) = 0;
      virtual HRESULT __stdcall RemoveBaseline ( ) = 0;
      virtual HRESULT __stdcall get_PasswordEncryptionProvider (
        /*[out,retval]*/ BSTR * PasswordEncryptionProvider ) = 0;
      virtual HRESULT __stdcall get_PasswordEncryptionAlgorithm (
        /*[out,retval]*/ BSTR * PasswordEncryptionAlgorithm ) = 0;
      virtual HRESULT __stdcall get_PasswordEncryptionKeyLength (
        /*[out,retval]*/ int * PasswordEncryptionKeyLength ) = 0;
      virtual HRESULT __stdcall get_PasswordEncryptionFileProperties (
        /*[out,retval]*/ VARIANT_BOOL * PasswordEncryptionFileProperties ) = 0;
      virtual HRESULT __stdcall SetPasswordEncryptionOptions (
        /*[in]*/ BSTR PasswordEncryptionProvider,
        /*[in]*/ BSTR PasswordEncryptionAlgorithm,
        /*[in]*/ int PasswordEncryptionKeyLength,
        /*[in]*/ VARIANT_BOOL PasswordEncryptionFileProperties ) = 0;
      virtual HRESULT __stdcall get_Password (
        /*[out,retval]*/ BSTR * Password ) = 0;
      virtual HRESULT __stdcall put_Password (
        /*[in]*/ BSTR Password ) = 0;
      virtual HRESULT __stdcall get_WritePassword (
        /*[out,retval]*/ BSTR * WritePassword ) = 0;
      virtual HRESULT __stdcall put_WritePassword (
        /*[in]*/ BSTR WritePassword ) = 0;
      virtual HRESULT __stdcall get_Permission (
        /*[out,retval]*/ struct Office2010::Permission * * Permission ) = 0;
      virtual HRESULT __stdcall get_SharedWorkspace (
        /*[out,retval]*/ struct Office2010::SharedWorkspace * * SharedWorkspace ) = 0;
      virtual HRESULT __stdcall get_Sync (
        /*[out,retval]*/ struct Office2010::Sync * * Sync ) = 0;
      virtual HRESULT __stdcall SendFaxOverInternet (
        /*[in]*/ BSTR Recipients,
        /*[in]*/ BSTR Subject,
        /*[in]*/ VARIANT_BOOL ShowMessage ) = 0;
      virtual HRESULT __stdcall get_DocumentLibraryVersions (
        /*[out,retval]*/ struct Office2010::DocumentLibraryVersions * * DocumentLibraryVersions ) = 0;
      virtual HRESULT __stdcall get_ContentTypeProperties (
        /*[out,retval]*/ struct Office2010::MetaProperties * * ContentTypeProperties ) = 0;
      virtual HRESULT __stdcall get_SectionCount (
        /*[out,retval]*/ int * SectionCount ) = 0;
      virtual HRESULT __stdcall get_HasSections (
        /*[out,retval]*/ VARIANT_BOOL * HasSections ) = 0;
      virtual HRESULT __stdcall NewSectionAfter (
        /*[in]*/ int Index,
        /*[in]*/ VARIANT_BOOL AfterSlide,
        /*[in]*/ BSTR sectionTitle,
        /*[out]*/ int * newSectionIndex ) = 0;
      virtual HRESULT __stdcall DeleteSection (
        /*[in]*/ int Index ) = 0;
      virtual HRESULT __stdcall DisableSections ( ) = 0;
      virtual HRESULT __stdcall sectionTitle (
        /*[in]*/ int Index,
        /*[out,retval]*/ BSTR * sectionTitle ) = 0;
      virtual HRESULT __stdcall RemoveDocumentInformation (
        /*[in]*/ enum PpRemoveDocInfoType Type ) = 0;
      virtual HRESULT __stdcall CheckInWithVersion (
        /*[in]*/ VARIANT_BOOL SaveChanges,
        /*[in]*/ VARIANT Comments = vtMissing,
        /*[in]*/ VARIANT MakePublic = vtMissing,
        /*[in]*/ VARIANT VersionType = vtMissing ) = 0;
      virtual HRESULT __stdcall ExportAsFixedFormat (
        /*[in]*/ BSTR Path,
        /*[in]*/ enum PpFixedFormatType FixedFormatType,
        /*[in]*/ enum PpFixedFormatIntent Intent,
        /*[in]*/ enum Office2010::MsoTriState FrameSlides,
        /*[in]*/ enum PpPrintHandoutOrder HandoutOrder,
        /*[in]*/ enum PpPrintOutputType OutputType,
        /*[in]*/ enum Office2010::MsoTriState PrintHiddenSlides,
        /*[in]*/ struct PrintRange * PrintRange,
        /*[in]*/ enum PpPrintRangeType RangeType,
        /*[in]*/ BSTR SlideShowName,
        /*[in]*/ VARIANT_BOOL IncludeDocProperties,
        /*[in]*/ VARIANT_BOOL KeepIRMSettings,
        /*[in]*/ VARIANT_BOOL DocStructureTags,
        /*[in]*/ VARIANT_BOOL BitmapMissingFonts,
        /*[in]*/ VARIANT_BOOL UseISO19005_1,
        /*[in]*/ VARIANT ExternalExporter = vtMissing ) = 0;
      virtual HRESULT __stdcall get_ServerPolicy (
        /*[out,retval]*/ struct Office2010::ServerPolicy * * ServerPolicy ) = 0;
      virtual HRESULT __stdcall GetWorkflowTasks (
        /*[out,retval]*/ struct Office2010::WorkflowTasks * * GetWorkflowTasks ) = 0;
      virtual HRESULT __stdcall GetWorkflowTemplates (
        /*[out,retval]*/ struct Office2010::WorkflowTemplates * * GetWorkflowTemplates ) = 0;
      virtual HRESULT __stdcall LockServerFile ( ) = 0;
      virtual HRESULT __stdcall get_DocumentInspectors (
        /*[out,retval]*/ struct Office2010::DocumentInspectors * * DocumentInspectors ) = 0;
      virtual HRESULT __stdcall get_HasVBProject (
        /*[out,retval]*/ VARIANT_BOOL * HasVBProject ) = 0;
      virtual HRESULT __stdcall get_CustomXMLParts (
        /*[out,retval]*/ struct Office2010::_CustomXMLParts * * CustomXMLParts ) = 0;
      virtual HRESULT __stdcall get_Final (
        /*[out,retval]*/ VARIANT_BOOL * Final ) = 0;
      virtual HRESULT __stdcall put_Final (
        /*[in]*/ VARIANT_BOOL Final ) = 0;
      virtual HRESULT __stdcall ApplyTheme (
        /*[in]*/ BSTR themeName ) = 0;
      virtual HRESULT __stdcall get_CustomerData (
        /*[out,retval]*/ struct CustomerData * * CustomerData ) = 0;
      virtual HRESULT __stdcall get_Research (
        /*[out,retval]*/ struct Research * * Research ) = 0;
      virtual HRESULT __stdcall PublishSlides (
        /*[in]*/ BSTR SlideLibraryUrl,
        /*[in]*/ VARIANT_BOOL Overwrite,
        /*[in]*/ VARIANT_BOOL UseSlideOrder ) = 0;
      virtual HRESULT __stdcall get_EncryptionProvider (
        /*[out,retval]*/ BSTR * EncryptionProvider ) = 0;
      virtual HRESULT __stdcall put_EncryptionProvider (
        /*[in]*/ BSTR EncryptionProvider ) = 0;
      virtual HRESULT __stdcall Convert ( ) = 0;
      virtual HRESULT __stdcall get_SectionProperties (
        /*[out,retval]*/ struct SectionProperties * * SectionProperties ) = 0;
      virtual HRESULT __stdcall get_Coauthoring (
        /*[out,retval]*/ struct Coauthoring * * Coauthoring ) = 0;
      virtual HRESULT __stdcall MergeWithBaseline (
        /*[in]*/ BSTR withPresentation,
        /*[in]*/ BSTR baselinePresentation ) = 0;
      virtual HRESULT __stdcall get_InMergeMode (
        /*[out,retval]*/ VARIANT_BOOL * InMergeMode ) = 0;
      virtual HRESULT __stdcall AcceptAll ( ) = 0;
      virtual HRESULT __stdcall RejectAll ( ) = 0;
      virtual HRESULT __stdcall EnsureAllMediaUpgraded ( ) = 0;
      virtual HRESULT __stdcall get_Broadcast (
        /*[out,retval]*/ struct Broadcast * * Broadcast ) = 0;
      virtual HRESULT __stdcall get_HasNotesMaster (
        /*[out,retval]*/ VARIANT_BOOL * HasNotesMaster ) = 0;
      virtual HRESULT __stdcall get_HasHandoutMaster (
        /*[out,retval]*/ VARIANT_BOOL * HasHandoutMaster ) = 0;
      virtual HRESULT __stdcall Convert2 (
        /*[in]*/ BSTR FileName ) = 0;
      virtual HRESULT __stdcall get_CreateVideoStatus (
        /*[out,retval]*/ enum PpMediaTaskStatus * CreateVideoStatus ) = 0;
      virtual HRESULT __stdcall CreateVideo (
        /*[in]*/ BSTR FileName,
        /*[in]*/ VARIANT_BOOL UseTimingsAndNarrations,
        /*[in]*/ int DefaultSlideDuration,
        /*[in]*/ int VertResolution,
        /*[in]*/ int FramesPerSecond,
        /*[in]*/ int Quality ) = 0;
};

struct __declspec(uuid("914934c9-5a91-11cf-8700-00aa0060263b"))
Cell : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Shape (
        /*[out,retval]*/ struct Shape * * Shape ) = 0;
      virtual HRESULT __stdcall get_Borders (
        /*[out,retval]*/ struct Borders * * Borders ) = 0;
      virtual HRESULT __stdcall Merge (
        /*[in]*/ struct Cell * MergeTo ) = 0;
      virtual HRESULT __stdcall Split (
        /*[in]*/ int NumRows,
        /*[in]*/ int NumColumns ) = 0;
      virtual HRESULT __stdcall Select ( ) = 0;
      virtual HRESULT __stdcall get_Selected (
        /*[out,retval]*/ VARIANT_BOOL * Selected ) = 0;
};

struct __declspec(uuid("914934c8-5a91-11cf-8700-00aa0060263b"))
CellRange : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct Cell * * Item ) = 0;
      virtual HRESULT __stdcall get_Borders (
        /*[out,retval]*/ struct Borders * * Borders ) = 0;
};

struct __declspec(uuid("914934c5-5a91-11cf-8700-00aa0060263b"))
Column : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Cells (
        /*[out,retval]*/ struct CellRange * * Cells ) = 0;
      virtual HRESULT __stdcall Select ( ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ float * Width ) = 0;
      virtual HRESULT __stdcall put_Width (
        /*[in]*/ float Width ) = 0;
};

struct __declspec(uuid("914934c4-5a91-11cf-8700-00aa0060263b"))
Columns : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct Column * * Item ) = 0;
      virtual HRESULT __stdcall Add (
        /*[in]*/ int BeforeColumn,
        /*[out,retval]*/ struct Column * * Add ) = 0;
};

struct __declspec(uuid("914934c7-5a91-11cf-8700-00aa0060263b"))
Row : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Cells (
        /*[out,retval]*/ struct CellRange * * Cells ) = 0;
      virtual HRESULT __stdcall Select ( ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ float * Height ) = 0;
      virtual HRESULT __stdcall put_Height (
        /*[in]*/ float Height ) = 0;
};

struct __declspec(uuid("914934c6-5a91-11cf-8700-00aa0060263b"))
Rows : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct Row * * Item ) = 0;
      virtual HRESULT __stdcall Add (
        /*[in]*/ int BeforeRow,
        /*[out,retval]*/ struct Row * * Add ) = 0;
};

struct __declspec(uuid("914934c3-5a91-11cf-8700-00aa0060263b"))
Table : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Columns (
        /*[out,retval]*/ struct Columns * * Columns ) = 0;
      virtual HRESULT __stdcall get_Rows (
        /*[out,retval]*/ struct Rows * * Rows ) = 0;
      virtual HRESULT __stdcall Cell (
        /*[in]*/ int Row,
        /*[in]*/ int Column,
        /*[out,retval]*/ struct Cell * * Cell ) = 0;
      virtual HRESULT __stdcall get_TableDirection (
        /*[out,retval]*/ enum PpDirection * TableDirection ) = 0;
      virtual HRESULT __stdcall put_TableDirection (
        /*[in]*/ enum PpDirection TableDirection ) = 0;
      virtual HRESULT __stdcall MergeBorders ( ) = 0;
      virtual HRESULT __stdcall get_FirstRow (
        /*[out,retval]*/ VARIANT_BOOL * FirstRow ) = 0;
      virtual HRESULT __stdcall put_FirstRow (
        /*[in]*/ VARIANT_BOOL FirstRow ) = 0;
      virtual HRESULT __stdcall get_LastRow (
        /*[out,retval]*/ VARIANT_BOOL * LastRow ) = 0;
      virtual HRESULT __stdcall put_LastRow (
        /*[in]*/ VARIANT_BOOL LastRow ) = 0;
      virtual HRESULT __stdcall get_FirstCol (
        /*[out,retval]*/ VARIANT_BOOL * FirstCol ) = 0;
      virtual HRESULT __stdcall put_FirstCol (
        /*[in]*/ VARIANT_BOOL FirstCol ) = 0;
      virtual HRESULT __stdcall get_LastCol (
        /*[out,retval]*/ VARIANT_BOOL * LastCol ) = 0;
      virtual HRESULT __stdcall put_LastCol (
        /*[in]*/ VARIANT_BOOL LastCol ) = 0;
      virtual HRESULT __stdcall get_HorizBanding (
        /*[out,retval]*/ VARIANT_BOOL * HorizBanding ) = 0;
      virtual HRESULT __stdcall put_HorizBanding (
        /*[in]*/ VARIANT_BOOL HorizBanding ) = 0;
      virtual HRESULT __stdcall get_VertBanding (
        /*[out,retval]*/ VARIANT_BOOL * VertBanding ) = 0;
      virtual HRESULT __stdcall put_VertBanding (
        /*[in]*/ VARIANT_BOOL VertBanding ) = 0;
      virtual HRESULT __stdcall get_Style (
        /*[out,retval]*/ struct TableStyle * * Style ) = 0;
      virtual HRESULT __stdcall get_Background (
        /*[out,retval]*/ struct TableBackground * * Background ) = 0;
      virtual HRESULT __stdcall ScaleProportionally (
        /*[in]*/ float scale ) = 0;
      virtual HRESULT __stdcall ApplyStyle (
        /*[in]*/ BSTR StyleID,
        /*[in]*/ VARIANT_BOOL SaveFormatting ) = 0;
      virtual HRESULT __stdcall get_AlternativeText (
        /*[out,retval]*/ BSTR * AlternativeText ) = 0;
      virtual HRESULT __stdcall put_AlternativeText (
        /*[in]*/ BSTR AlternativeText ) = 0;
      virtual HRESULT __stdcall get_Title (
        /*[out,retval]*/ BSTR * Title ) = 0;
      virtual HRESULT __stdcall put_Title (
        /*[in]*/ BSTR Title ) = 0;
};

struct __declspec(uuid("914934e0-5a91-11cf-8700-00aa0060263b"))
Timing : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Duration (
        /*[out,retval]*/ float * Duration ) = 0;
      virtual HRESULT __stdcall put_Duration (
        /*[in]*/ float Duration ) = 0;
      virtual HRESULT __stdcall get_TriggerType (
        /*[out,retval]*/ enum MsoAnimTriggerType * TriggerType ) = 0;
      virtual HRESULT __stdcall put_TriggerType (
        /*[in]*/ enum MsoAnimTriggerType TriggerType ) = 0;
      virtual HRESULT __stdcall get_TriggerDelayTime (
        /*[out,retval]*/ float * TriggerDelayTime ) = 0;
      virtual HRESULT __stdcall put_TriggerDelayTime (
        /*[in]*/ float TriggerDelayTime ) = 0;
      virtual HRESULT __stdcall get_TriggerShape (
        /*[out,retval]*/ struct Shape * * TriggerShape ) = 0;
      virtual HRESULT __stdcall put_TriggerShape (
        /*[in]*/ struct Shape * TriggerShape ) = 0;
      virtual HRESULT __stdcall get_RepeatCount (
        /*[out,retval]*/ int * RepeatCount ) = 0;
      virtual HRESULT __stdcall put_RepeatCount (
        /*[in]*/ int RepeatCount ) = 0;
      virtual HRESULT __stdcall get_RepeatDuration (
        /*[out,retval]*/ float * RepeatDuration ) = 0;
      virtual HRESULT __stdcall put_RepeatDuration (
        /*[in]*/ float RepeatDuration ) = 0;
      virtual HRESULT __stdcall get_Speed (
        /*[out,retval]*/ float * Speed ) = 0;
      virtual HRESULT __stdcall put_Speed (
        /*[in]*/ float Speed ) = 0;
      virtual HRESULT __stdcall get_Accelerate (
        /*[out,retval]*/ float * Accelerate ) = 0;
      virtual HRESULT __stdcall put_Accelerate (
        /*[in]*/ float Accelerate ) = 0;
      virtual HRESULT __stdcall get_Decelerate (
        /*[out,retval]*/ float * Decelerate ) = 0;
      virtual HRESULT __stdcall put_Decelerate (
        /*[in]*/ float Decelerate ) = 0;
      virtual HRESULT __stdcall get_AutoReverse (
        /*[out,retval]*/ enum Office2010::MsoTriState * AutoReverse ) = 0;
      virtual HRESULT __stdcall put_AutoReverse (
        /*[in]*/ enum Office2010::MsoTriState AutoReverse ) = 0;
      virtual HRESULT __stdcall get_SmoothStart (
        /*[out,retval]*/ enum Office2010::MsoTriState * SmoothStart ) = 0;
      virtual HRESULT __stdcall put_SmoothStart (
        /*[in]*/ enum Office2010::MsoTriState SmoothStart ) = 0;
      virtual HRESULT __stdcall get_SmoothEnd (
        /*[out,retval]*/ enum Office2010::MsoTriState * SmoothEnd ) = 0;
      virtual HRESULT __stdcall put_SmoothEnd (
        /*[in]*/ enum Office2010::MsoTriState SmoothEnd ) = 0;
      virtual HRESULT __stdcall get_RewindAtEnd (
        /*[out,retval]*/ enum Office2010::MsoTriState * RewindAtEnd ) = 0;
      virtual HRESULT __stdcall put_RewindAtEnd (
        /*[in]*/ enum Office2010::MsoTriState RewindAtEnd ) = 0;
      virtual HRESULT __stdcall get_Restart (
        /*[out,retval]*/ enum MsoAnimEffectRestart * Restart ) = 0;
      virtual HRESULT __stdcall put_Restart (
        /*[in]*/ enum MsoAnimEffectRestart Restart ) = 0;
      virtual HRESULT __stdcall get_BounceEnd (
        /*[out,retval]*/ enum Office2010::MsoTriState * BounceEnd ) = 0;
      virtual HRESULT __stdcall put_BounceEnd (
        /*[in]*/ enum Office2010::MsoTriState BounceEnd ) = 0;
      virtual HRESULT __stdcall get_BounceEndIntensity (
        /*[out,retval]*/ float * BounceEndIntensity ) = 0;
      virtual HRESULT __stdcall put_BounceEndIntensity (
        /*[in]*/ float BounceEndIntensity ) = 0;
      virtual HRESULT __stdcall get_TriggerBookmark (
        /*[out,retval]*/ BSTR * TriggerBookmark ) = 0;
      virtual HRESULT __stdcall put_TriggerBookmark (
        /*[in]*/ BSTR TriggerBookmark ) = 0;
};

struct __declspec(uuid("914934e4-5a91-11cf-8700-00aa0060263b"))
AnimationBehavior : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Additive (
        /*[out,retval]*/ enum MsoAnimAdditive * Additive ) = 0;
      virtual HRESULT __stdcall put_Additive (
        /*[in]*/ enum MsoAnimAdditive Additive ) = 0;
      virtual HRESULT __stdcall get_Accumulate (
        /*[out,retval]*/ enum MsoAnimAccumulate * Accumulate ) = 0;
      virtual HRESULT __stdcall put_Accumulate (
        /*[in]*/ enum MsoAnimAccumulate Accumulate ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum MsoAnimType * Type ) = 0;
      virtual HRESULT __stdcall put_Type (
        /*[in]*/ enum MsoAnimType Type ) = 0;
      virtual HRESULT __stdcall get_MotionEffect (
        /*[out,retval]*/ struct MotionEffect * * MotionEffect ) = 0;
      virtual HRESULT __stdcall get_ColorEffect (
        /*[out,retval]*/ struct ColorEffect * * ColorEffect ) = 0;
      virtual HRESULT __stdcall get_ScaleEffect (
        /*[out,retval]*/ struct ScaleEffect * * ScaleEffect ) = 0;
      virtual HRESULT __stdcall get_RotationEffect (
        /*[out,retval]*/ struct RotationEffect * * RotationEffect ) = 0;
      virtual HRESULT __stdcall get_PropertyEffect (
        /*[out,retval]*/ struct PropertyEffect * * PropertyEffect ) = 0;
      virtual HRESULT __stdcall get_Timing (
        /*[out,retval]*/ struct Timing * * Timing ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
      virtual HRESULT __stdcall get_CommandEffect (
        /*[out,retval]*/ struct CommandEffect * * CommandEffect ) = 0;
      virtual HRESULT __stdcall get_FilterEffect (
        /*[out,retval]*/ struct FilterEffect * * FilterEffect ) = 0;
      virtual HRESULT __stdcall get_SetEffect (
        /*[out,retval]*/ struct SetEffect * * SetEffect ) = 0;
};

struct __declspec(uuid("914934e3-5a91-11cf-8700-00aa0060263b"))
AnimationBehaviors : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct AnimationBehavior * * Item ) = 0;
      virtual HRESULT __stdcall Add (
        /*[in]*/ enum MsoAnimType Type,
        /*[in]*/ int Index,
        /*[out,retval]*/ struct AnimationBehavior * * Add ) = 0;
};

struct __declspec(uuid("914934df-5a91-11cf-8700-00aa0060263b"))
Effect : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Shape (
        /*[out,retval]*/ struct Shape * * Shape ) = 0;
      virtual HRESULT __stdcall put_Shape (
        /*[in]*/ struct Shape * Shape ) = 0;
      virtual HRESULT __stdcall MoveTo (
        /*[in]*/ int toPos ) = 0;
      virtual HRESULT __stdcall MoveBefore (
        /*[in]*/ struct Effect * Effect ) = 0;
      virtual HRESULT __stdcall MoveAfter (
        /*[in]*/ struct Effect * Effect ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
      virtual HRESULT __stdcall get_Index (
        /*[out,retval]*/ int * Index ) = 0;
      virtual HRESULT __stdcall get_Timing (
        /*[out,retval]*/ struct Timing * * Timing ) = 0;
      virtual HRESULT __stdcall get_EffectType (
        /*[out,retval]*/ enum MsoAnimEffect * EffectType ) = 0;
      virtual HRESULT __stdcall put_EffectType (
        /*[in]*/ enum MsoAnimEffect EffectType ) = 0;
      virtual HRESULT __stdcall get_EffectParameters (
        /*[out,retval]*/ struct EffectParameters * * EffectParameters ) = 0;
      virtual HRESULT __stdcall get_TextRangeStart (
        /*[out,retval]*/ int * TextRangeStart ) = 0;
      virtual HRESULT __stdcall get_TextRangeLength (
        /*[out,retval]*/ int * TextRangeLength ) = 0;
      virtual HRESULT __stdcall get_Paragraph (
        /*[out,retval]*/ int * Paragraph ) = 0;
      virtual HRESULT __stdcall put_Paragraph (
        /*[in]*/ int Paragraph ) = 0;
      virtual HRESULT __stdcall get_DisplayName (
        /*[out,retval]*/ BSTR * DisplayName ) = 0;
      virtual HRESULT __stdcall get_Exit (
        /*[out,retval]*/ enum Office2010::MsoTriState * Exit ) = 0;
      virtual HRESULT __stdcall put_Exit (
        /*[in]*/ enum Office2010::MsoTriState Exit ) = 0;
      virtual HRESULT __stdcall get_Behaviors (
        /*[out,retval]*/ struct AnimationBehaviors * * Behaviors ) = 0;
      virtual HRESULT __stdcall get_EffectInformation (
        /*[out,retval]*/ struct EffectInformation * * EffectInformation ) = 0;
};

struct __declspec(uuid("914934c2-5a91-11cf-8700-00aa0060263b"))
EApplication : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall WindowSelectionChange (
        /*[in]*/ struct Selection * Sel ) = 0;
      virtual HRESULT __stdcall WindowBeforeRightClick (
        /*[in]*/ struct Selection * Sel,
        /*[in,out]*/ VARIANT_BOOL * Cancel ) = 0;
      virtual HRESULT __stdcall WindowBeforeDoubleClick (
        /*[in]*/ struct Selection * Sel,
        /*[in,out]*/ VARIANT_BOOL * Cancel ) = 0;
      virtual HRESULT __stdcall PresentationClose (
        /*[in]*/ struct _Presentation * Pres ) = 0;
      virtual HRESULT __stdcall PresentationSave (
        /*[in]*/ struct _Presentation * Pres ) = 0;
      virtual HRESULT __stdcall PresentationOpen (
        /*[in]*/ struct _Presentation * Pres ) = 0;
      virtual HRESULT __stdcall NewPresentation (
        /*[in]*/ struct _Presentation * Pres ) = 0;
      virtual HRESULT __stdcall PresentationNewSlide (
        /*[in]*/ struct _Slide * Sld ) = 0;
      virtual HRESULT __stdcall WindowActivate (
        /*[in]*/ struct _Presentation * Pres,
        /*[in]*/ struct DocumentWindow * Wn ) = 0;
      virtual HRESULT __stdcall WindowDeactivate (
        /*[in]*/ struct _Presentation * Pres,
        /*[in]*/ struct DocumentWindow * Wn ) = 0;
      virtual HRESULT __stdcall SlideShowBegin (
        /*[in]*/ struct SlideShowWindow * Wn ) = 0;
      virtual HRESULT __stdcall SlideShowNextBuild (
        /*[in]*/ struct SlideShowWindow * Wn ) = 0;
      virtual HRESULT __stdcall SlideShowNextSlide (
        /*[in]*/ struct SlideShowWindow * Wn ) = 0;
      virtual HRESULT __stdcall SlideShowEnd (
        /*[in]*/ struct _Presentation * Pres ) = 0;
      virtual HRESULT __stdcall PresentationPrint (
        /*[in]*/ struct _Presentation * Pres ) = 0;
      virtual HRESULT __stdcall SlideSelectionChanged (
        /*[in]*/ struct SlideRange * SldRange ) = 0;
      virtual HRESULT __stdcall ColorSchemeChanged (
        /*[in]*/ struct SlideRange * SldRange ) = 0;
      virtual HRESULT __stdcall PresentationBeforeSave (
        /*[in]*/ struct _Presentation * Pres,
        /*[in,out]*/ VARIANT_BOOL * Cancel ) = 0;
      virtual HRESULT __stdcall SlideShowNextClick (
        /*[in]*/ struct SlideShowWindow * Wn,
        /*[in]*/ struct Effect * nEffect ) = 0;
      virtual HRESULT __stdcall AfterNewPresentation (
        /*[in]*/ struct _Presentation * Pres ) = 0;
      virtual HRESULT __stdcall AfterPresentationOpen (
        /*[in]*/ struct _Presentation * Pres ) = 0;
      virtual HRESULT __stdcall PresentationSync (
        /*[in]*/ struct _Presentation * Pres,
        /*[in]*/ enum Office2010::MsoSyncEventType SyncEventType ) = 0;
      virtual HRESULT __stdcall SlideShowOnNext (
        /*[in]*/ struct SlideShowWindow * Wn ) = 0;
      virtual HRESULT __stdcall SlideShowOnPrevious (
        /*[in]*/ struct SlideShowWindow * Wn ) = 0;
      virtual HRESULT __stdcall PresentationBeforeClose (
        /*[in]*/ struct _Presentation * Pres,
        /*[in,out]*/ VARIANT_BOOL * Cancel ) = 0;
      virtual HRESULT __stdcall ProtectedViewWindowOpen (
        /*[in]*/ struct ProtectedViewWindow * ProtViewWindow ) = 0;
      virtual HRESULT __stdcall ProtectedViewWindowBeforeEdit (
        /*[in]*/ struct ProtectedViewWindow * ProtViewWindow,
        /*[in,out]*/ VARIANT_BOOL * Cancel ) = 0;
      virtual HRESULT __stdcall ProtectedViewWindowBeforeClose (
        /*[in]*/ struct ProtectedViewWindow * ProtViewWindow,
        /*[in]*/ enum PpProtectedViewCloseReason ProtectedViewCloseReason,
        /*[in,out]*/ VARIANT_BOOL * Cancel ) = 0;
      virtual HRESULT __stdcall ProtectedViewWindowActivate (
        /*[in]*/ struct ProtectedViewWindow * ProtViewWindow ) = 0;
      virtual HRESULT __stdcall ProtectedViewWindowDeactivate (
        /*[in]*/ struct ProtectedViewWindow * ProtViewWindow ) = 0;
      virtual HRESULT __stdcall PresentationCloseFinal (
        /*[in]*/ struct _Presentation * Pres ) = 0;
};

struct __declspec(uuid("914934de-5a91-11cf-8700-00aa0060263b"))
Sequence : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct Effect * * Item ) = 0;
      virtual HRESULT __stdcall AddEffect (
        /*[in]*/ struct Shape * Shape,
        /*[in]*/ enum MsoAnimEffect effectId,
        /*[in]*/ enum MsoAnimateByLevel Level,
        /*[in]*/ enum MsoAnimTriggerType trigger,
        /*[in]*/ int Index,
        /*[out,retval]*/ struct Effect * * Effect ) = 0;
      virtual HRESULT __stdcall Clone (
        /*[in]*/ struct Effect * Effect,
        /*[in]*/ int Index,
        /*[out,retval]*/ struct Effect * * Clone ) = 0;
      virtual HRESULT __stdcall FindFirstAnimationFor (
        /*[in]*/ struct Shape * Shape,
        /*[out,retval]*/ struct Effect * * FindFirstAnimationFor ) = 0;
      virtual HRESULT __stdcall FindFirstAnimationForClick (
        /*[in]*/ int click,
        /*[out,retval]*/ struct Effect * * FindFirstAnimationForClick ) = 0;
      virtual HRESULT __stdcall ConvertToBuildLevel (
        /*[in]*/ struct Effect * Effect,
        /*[in]*/ enum MsoAnimateByLevel Level,
        /*[out,retval]*/ struct Effect * * ConvertToBuildLevel ) = 0;
      virtual HRESULT __stdcall ConvertToAfterEffect (
        /*[in]*/ struct Effect * Effect,
        /*[in]*/ enum MsoAnimAfterEffect After,
        /*[in]*/ Office2010::MsoRGBType DimColor,
        /*[in]*/ enum PpColorSchemeIndex DimSchemeColor,
        /*[out,retval]*/ struct Effect * * ConvertToAfterEffect ) = 0;
      virtual HRESULT __stdcall ConvertToAnimateBackground (
        /*[in]*/ struct Effect * Effect,
        /*[in]*/ enum Office2010::MsoTriState AnimateBackground,
        /*[out,retval]*/ struct Effect * * ConvertToAnimateBackground ) = 0;
      virtual HRESULT __stdcall ConvertToAnimateInReverse (
        /*[in]*/ struct Effect * Effect,
        /*[in]*/ enum Office2010::MsoTriState animateInReverse,
        /*[out,retval]*/ struct Effect * * ConvertToAnimateInReverse ) = 0;
      virtual HRESULT __stdcall ConvertToTextUnitEffect (
        /*[in]*/ struct Effect * Effect,
        /*[in]*/ enum MsoAnimTextUnitEffect unitEffect,
        /*[out,retval]*/ struct Effect * * ConvertToTextUnitEffect ) = 0;
      virtual HRESULT __stdcall AddTriggerEffect (
        /*[in]*/ struct Shape * pShape,
        /*[in]*/ enum MsoAnimEffect effectId,
        /*[in]*/ enum MsoAnimTriggerType trigger,
        /*[in]*/ struct Shape * pTriggerShape,
        /*[in]*/ BSTR bookmark,
        /*[in]*/ enum MsoAnimateByLevel Level,
        /*[out,retval]*/ struct Effect * * TriggerEffect ) = 0;
};

struct __declspec(uuid("914934dd-5a91-11cf-8700-00aa0060263b"))
Sequences : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct Sequence * * Item ) = 0;
      virtual HRESULT __stdcall Add (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct Sequence * * Add ) = 0;
};

struct __declspec(uuid("914934dc-5a91-11cf-8700-00aa0060263b"))
TimeLine : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_MainSequence (
        /*[out,retval]*/ struct Sequence * * MainSequence ) = 0;
      virtual HRESULT __stdcall get_InteractiveSequences (
        /*[out,retval]*/ struct Sequences * * InteractiveSequences ) = 0;
};

struct __declspec(uuid("ba72e553-4ff5-48f4-8215-5505f990966f"))
ResampleMediaTask : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_profile (
        /*[out,retval]*/ enum PpResampleMediaProfile * profile ) = 0;
      virtual HRESULT __stdcall get_IsLinked (
        /*[out,retval]*/ VARIANT_BOOL * IsLinked ) = 0;
      virtual HRESULT __stdcall get_IsEmbedded (
        /*[out,retval]*/ VARIANT_BOOL * IsEmbedded ) = 0;
      virtual HRESULT __stdcall get_AudioSamplingRate (
        /*[out,retval]*/ long * AudioSamplingRate ) = 0;
      virtual HRESULT __stdcall get_VideoFrameRate (
        /*[out,retval]*/ long * VideoFrameRate ) = 0;
      virtual HRESULT __stdcall get_SampleHeight (
        /*[out,retval]*/ long * SampleHeight ) = 0;
      virtual HRESULT __stdcall get_SampleWidth (
        /*[out,retval]*/ long * SampleWidth ) = 0;
      virtual HRESULT __stdcall get_ContainerType (
        /*[out,retval]*/ BSTR * ContainerType ) = 0;
      virtual HRESULT __stdcall get_VideoCompressionType (
        /*[out,retval]*/ BSTR * VideoCompressionType ) = 0;
      virtual HRESULT __stdcall get_AudioCompressionType (
        /*[out,retval]*/ BSTR * AudioCompressionType ) = 0;
      virtual HRESULT __stdcall get_Shape (
        /*[out,retval]*/ struct Shape * * Shape ) = 0;
};

struct __declspec(uuid("ba72e554-4ff5-48f4-8215-5505f990966f"))
ResampleMediaTasks : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall Item (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct ResampleMediaTask * * Item ) = 0;
      virtual HRESULT __stdcall Pause ( ) = 0;
      virtual HRESULT __stdcall Cancel ( ) = 0;
      virtual HRESULT __stdcall Resume ( ) = 0;
      virtual HRESULT __stdcall get_PercentComplete (
        /*[out,retval]*/ long * PercentComplete ) = 0;
};

struct __declspec(uuid("9149347a-5a91-11cf-8700-00aa0060263b"))
ShapeRange : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Apply ( ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
      virtual HRESULT __stdcall Flip (
        /*[in]*/ enum Office2010::MsoFlipCmd FlipCmd ) = 0;
      virtual HRESULT __stdcall IncrementLeft (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall IncrementRotation (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall IncrementTop (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall PickUp ( ) = 0;
      virtual HRESULT __stdcall RerouteConnections ( ) = 0;
      virtual HRESULT __stdcall ScaleHeight (
        /*[in]*/ float Factor,
        /*[in]*/ enum Office2010::MsoTriState RelativeToOriginalSize,
        /*[in]*/ enum Office2010::MsoScaleFrom fScale ) = 0;
      virtual HRESULT __stdcall ScaleWidth (
        /*[in]*/ float Factor,
        /*[in]*/ enum Office2010::MsoTriState RelativeToOriginalSize,
        /*[in]*/ enum Office2010::MsoScaleFrom fScale ) = 0;
      virtual HRESULT __stdcall SetShapesDefaultProperties ( ) = 0;
      virtual HRESULT __stdcall Ungroup (
        /*[out,retval]*/ struct ShapeRange * * Ungroup ) = 0;
      virtual HRESULT __stdcall ZOrder (
        /*[in]*/ enum Office2010::MsoZOrderCmd ZOrderCmd ) = 0;
      virtual HRESULT __stdcall get_Adjustments (
        /*[out,retval]*/ struct Adjustments * * Adjustments ) = 0;
      virtual HRESULT __stdcall get_AutoShapeType (
        /*[out,retval]*/ enum Office2010::MsoAutoShapeType * AutoShapeType ) = 0;
      virtual HRESULT __stdcall put_AutoShapeType (
        /*[in]*/ enum Office2010::MsoAutoShapeType AutoShapeType ) = 0;
      virtual HRESULT __stdcall get_BlackWhiteMode (
        /*[out,retval]*/ enum Office2010::MsoBlackWhiteMode * BlackWhiteMode ) = 0;
      virtual HRESULT __stdcall put_BlackWhiteMode (
        /*[in]*/ enum Office2010::MsoBlackWhiteMode BlackWhiteMode ) = 0;
      virtual HRESULT __stdcall get_Callout (
        /*[out,retval]*/ struct CalloutFormat * * Callout ) = 0;
      virtual HRESULT __stdcall get_ConnectionSiteCount (
        /*[out,retval]*/ int * ConnectionSiteCount ) = 0;
      virtual HRESULT __stdcall get_Connector (
        /*[out,retval]*/ enum Office2010::MsoTriState * Connector ) = 0;
      virtual HRESULT __stdcall get_ConnectorFormat (
        /*[out,retval]*/ struct ConnectorFormat * * ConnectorFormat ) = 0;
      virtual HRESULT __stdcall get_Fill (
        /*[out,retval]*/ struct FillFormat * * Fill ) = 0;
      virtual HRESULT __stdcall get_GroupItems (
        /*[out,retval]*/ struct GroupShapes * * GroupItems ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ float * Height ) = 0;
      virtual HRESULT __stdcall put_Height (
        /*[in]*/ float Height ) = 0;
      virtual HRESULT __stdcall get_HorizontalFlip (
        /*[out,retval]*/ enum Office2010::MsoTriState * HorizontalFlip ) = 0;
      virtual HRESULT __stdcall get_Left (
        /*[out,retval]*/ float * Left ) = 0;
      virtual HRESULT __stdcall put_Left (
        /*[in]*/ float Left ) = 0;
      virtual HRESULT __stdcall get_Line (
        /*[out,retval]*/ struct LineFormat * * Line ) = 0;
      virtual HRESULT __stdcall get_LockAspectRatio (
        /*[out,retval]*/ enum Office2010::MsoTriState * LockAspectRatio ) = 0;
      virtual HRESULT __stdcall put_LockAspectRatio (
        /*[in]*/ enum Office2010::MsoTriState LockAspectRatio ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall put_Name (
        /*[in]*/ BSTR Name ) = 0;
      virtual HRESULT __stdcall get_Nodes (
        /*[out,retval]*/ struct ShapeNodes * * Nodes ) = 0;
      virtual HRESULT __stdcall get_Rotation (
        /*[out,retval]*/ float * Rotation ) = 0;
      virtual HRESULT __stdcall put_Rotation (
        /*[in]*/ float Rotation ) = 0;
      virtual HRESULT __stdcall get_PictureFormat (
        /*[out,retval]*/ struct PictureFormat * * Picture ) = 0;
      virtual HRESULT __stdcall get_Shadow (
        /*[out,retval]*/ struct ShadowFormat * * Shadow ) = 0;
      virtual HRESULT __stdcall get_TextEffect (
        /*[out,retval]*/ struct TextEffectFormat * * TextEffect ) = 0;
      virtual HRESULT __stdcall get_TextFrame (
        /*[out,retval]*/ struct TextFrame * * TextFrame ) = 0;
      virtual HRESULT __stdcall get_ThreeD (
        /*[out,retval]*/ struct ThreeDFormat * * ThreeD ) = 0;
      virtual HRESULT __stdcall get_Top (
        /*[out,retval]*/ float * Top ) = 0;
      virtual HRESULT __stdcall put_Top (
        /*[in]*/ float Top ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum Office2010::MsoShapeType * Type ) = 0;
      virtual HRESULT __stdcall get_VerticalFlip (
        /*[out,retval]*/ enum Office2010::MsoTriState * VerticalFlip ) = 0;
      virtual HRESULT __stdcall get_Vertices (
        /*[out,retval]*/ VARIANT * Vertices ) = 0;
      virtual HRESULT __stdcall get_Visible (
        /*[out,retval]*/ enum Office2010::MsoTriState * Visible ) = 0;
      virtual HRESULT __stdcall put_Visible (
        /*[in]*/ enum Office2010::MsoTriState Visible ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ float * Width ) = 0;
      virtual HRESULT __stdcall put_Width (
        /*[in]*/ float Width ) = 0;
      virtual HRESULT __stdcall get_ZOrderPosition (
        /*[out,retval]*/ int * ZOrderPosition ) = 0;
      virtual HRESULT __stdcall get_OLEFormat (
        /*[out,retval]*/ struct OLEFormat * * OLEFormat ) = 0;
      virtual HRESULT __stdcall get_LinkFormat (
        /*[out,retval]*/ struct LinkFormat * * LinkFormat ) = 0;
      virtual HRESULT __stdcall get_PlaceholderFormat (
        /*[out,retval]*/ struct PlaceholderFormat * * PlaceholderFormat ) = 0;
      virtual HRESULT __stdcall get_AnimationSettings (
        /*[out,retval]*/ struct AnimationSettings * * AnimationSettings ) = 0;
      virtual HRESULT __stdcall get_ActionSettings (
        /*[out,retval]*/ struct ActionSettings * * ActionSettings ) = 0;
      virtual HRESULT __stdcall get_Tags (
        /*[out,retval]*/ struct Tags * * Tags ) = 0;
      virtual HRESULT __stdcall Cut ( ) = 0;
      virtual HRESULT __stdcall Copy ( ) = 0;
      virtual HRESULT __stdcall Select (
        /*[in]*/ enum Office2010::MsoTriState Replace ) = 0;
      virtual HRESULT __stdcall Duplicate (
        /*[out,retval]*/ struct ShapeRange * * Duplicate ) = 0;
      virtual HRESULT __stdcall get_MediaType (
        /*[out,retval]*/ enum PpMediaType * MediaType ) = 0;
      virtual HRESULT __stdcall get_HasTextFrame (
        /*[out,retval]*/ enum Office2010::MsoTriState * HasTextFrame ) = 0;
      virtual HRESULT __stdcall get_SoundFormat (
        /*[out,retval]*/ struct SoundFormat * * SoundFormat ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct Shape * * Item ) = 0;
      virtual HRESULT __stdcall get__NewEnum (
        /*[out,retval]*/ IUnknown * * _NewEnum ) = 0;
      virtual HRESULT __stdcall _Index (
        /*[in]*/ int Index,
        /*[out,retval]*/ VARIANT * _Index ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ long * Count ) = 0;
      virtual HRESULT __stdcall Group (
        /*[out,retval]*/ struct Shape * * Group ) = 0;
      virtual HRESULT __stdcall Regroup (
        /*[out,retval]*/ struct Shape * * Regroup ) = 0;
      virtual HRESULT __stdcall Align (
        /*[in]*/ enum Office2010::MsoAlignCmd AlignCmd,
        /*[in]*/ enum Office2010::MsoTriState RelativeTo ) = 0;
      virtual HRESULT __stdcall Distribute (
        /*[in]*/ enum Office2010::MsoDistributeCmd DistributeCmd,
        /*[in]*/ enum Office2010::MsoTriState RelativeTo ) = 0;
      virtual HRESULT __stdcall GetPolygonalRepresentation (
        /*[in]*/ unsigned long maxPointsInBuffer,
        /*[in]*/ float * pPoints,
        /*[out]*/ unsigned long * numPointsInPolygon,
        /*[out]*/ enum Office2010::MsoTriState * IsOpen ) = 0;
      virtual HRESULT __stdcall get_Script (
        /*[out,retval]*/ struct Office2010::Script * * Script ) = 0;
      virtual HRESULT __stdcall get_AlternativeText (
        /*[out,retval]*/ BSTR * AlternativeText ) = 0;
      virtual HRESULT __stdcall put_AlternativeText (
        /*[in]*/ BSTR AlternativeText ) = 0;
      virtual HRESULT __stdcall get_HasTable (
        /*[out,retval]*/ enum Office2010::MsoTriState * HasTable ) = 0;
      virtual HRESULT __stdcall get_Table (
        /*[out,retval]*/ struct Table * * Table ) = 0;
      virtual HRESULT __stdcall Export (
        /*[in]*/ BSTR PathName,
        /*[in]*/ enum PpShapeFormat Filter,
        /*[in]*/ int ScaleWidth,
        /*[in]*/ int ScaleHeight,
        /*[in]*/ enum PpExportMode ExportMode ) = 0;
      virtual HRESULT __stdcall get_HasDiagram (
        /*[out,retval]*/ enum Office2010::MsoTriState * pHasDiagram ) = 0;
      virtual HRESULT __stdcall get_Diagram (
        /*[out,retval]*/ struct Diagram * * Diagram ) = 0;
      virtual HRESULT __stdcall get_HasDiagramNode (
        /*[out,retval]*/ enum Office2010::MsoTriState * pHasDiagram ) = 0;
      virtual HRESULT __stdcall get_DiagramNode (
        /*[out,retval]*/ struct DiagramNode * * DiagramNode ) = 0;
      virtual HRESULT __stdcall get_Child (
        /*[out,retval]*/ enum Office2010::MsoTriState * Child ) = 0;
      virtual HRESULT __stdcall get_ParentGroup (
        /*[out,retval]*/ struct Shape * * Parent ) = 0;
      virtual HRESULT __stdcall get_CanvasItems (
        /*[out,retval]*/ struct CanvasShapes * * CanvasShapes ) = 0;
      virtual HRESULT __stdcall get_Id (
        /*[out,retval]*/ int * pid ) = 0;
      virtual HRESULT __stdcall CanvasCropLeft (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall CanvasCropTop (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall CanvasCropRight (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall CanvasCropBottom (
        /*[in]*/ float Increment ) = 0;
      virtual HRESULT __stdcall put_RTF (
        /*[in]*/ BSTR _arg1 ) = 0;
      virtual HRESULT __stdcall get_CustomerData (
        /*[out,retval]*/ struct CustomerData * * CustomerData ) = 0;
      virtual HRESULT __stdcall get_TextFrame2 (
        /*[out,retval]*/ struct TextFrame2 * * Frame ) = 0;
      virtual HRESULT __stdcall get_HasChart (
        /*[out,retval]*/ enum Office2010::MsoTriState * pHasChart ) = 0;
      virtual HRESULT __stdcall get_ShapeStyle (
        /*[out,retval]*/ enum Office2010::MsoShapeStyleIndex * ShapeStyle ) = 0;
      virtual HRESULT __stdcall put_ShapeStyle (
        /*[in]*/ enum Office2010::MsoShapeStyleIndex ShapeStyle ) = 0;
      virtual HRESULT __stdcall get_BackgroundStyle (
        /*[out,retval]*/ enum Office2010::MsoBackgroundStyleIndex * BackgroundStyle ) = 0;
      virtual HRESULT __stdcall put_BackgroundStyle (
        /*[in]*/ enum Office2010::MsoBackgroundStyleIndex BackgroundStyle ) = 0;
      virtual HRESULT __stdcall get_SoftEdge (
        /*[out,retval]*/ struct Office2010::SoftEdgeFormat * * SoftEdge ) = 0;
      virtual HRESULT __stdcall get_Glow (
        /*[out,retval]*/ struct Office2010::GlowFormat * * Glow ) = 0;
      virtual HRESULT __stdcall get_Reflection (
        /*[out,retval]*/ struct Office2010::ReflectionFormat * * Reflection ) = 0;
      virtual HRESULT __stdcall get_Chart (
        /*[out,retval]*/ struct Chart * * Chart ) = 0;
      virtual HRESULT __stdcall get_HasSmartArt (
        /*[out,retval]*/ enum Office2010::MsoTriState * HasSmartArt ) = 0;
      virtual HRESULT __stdcall get_SmartArt (
        /*[out,retval]*/ struct Office2010::SmartArt * * SmartArt ) = 0;
      virtual HRESULT __stdcall ConvertTextToSmartArt (
        /*[in]*/ struct Office2010::SmartArtLayout * Layout ) = 0;
      virtual HRESULT __stdcall get_Title (
        /*[out,retval]*/ BSTR * Title ) = 0;
      virtual HRESULT __stdcall put_Title (
        /*[in]*/ BSTR Title ) = 0;
      virtual HRESULT __stdcall get_MediaFormat (
        /*[out,retval]*/ struct MediaFormat * * MediaFormat ) = 0;
      virtual HRESULT __stdcall PickupAnimation ( ) = 0;
      virtual HRESULT __stdcall ApplyAnimation ( ) = 0;
      virtual HRESULT __stdcall UpgradeMedia ( ) = 0;
};

struct __declspec(uuid("9149347b-5a91-11cf-8700-00aa0060263b"))
GroupShapes : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ int * pnShapes ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct Shape * * Item ) = 0;
      virtual HRESULT __stdcall get__NewEnum (
        /*[out,retval]*/ IUnknown * * ppienum ) = 0;
      virtual HRESULT __stdcall Range (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct ShapeRange * * Range ) = 0;
};

struct __declspec(uuid("914934ec-5a91-11cf-8700-00aa0060263b"))
CanvasShapes : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ int * Count ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct Shape * * Item ) = 0;
      virtual HRESULT __stdcall get__NewEnum (
        /*[out,retval]*/ IUnknown * * _NewEnum ) = 0;
      virtual HRESULT __stdcall AddCallout (
        /*[in]*/ enum Office2010::MsoCalloutType Type,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * Callout ) = 0;
      virtual HRESULT __stdcall AddConnector (
        /*[in]*/ enum Office2010::MsoConnectorType Type,
        /*[in]*/ float BeginX,
        /*[in]*/ float BeginY,
        /*[in]*/ float EndX,
        /*[in]*/ float EndY,
        /*[out,retval]*/ struct Shape * * Connector ) = 0;
      virtual HRESULT __stdcall AddCurve (
        /*[in]*/ VARIANT SafeArrayOfPoints,
        /*[out,retval]*/ struct Shape * * Curve ) = 0;
      virtual HRESULT __stdcall AddLabel (
        /*[in]*/ enum Office2010::MsoTextOrientation Orientation,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * Label ) = 0;
      virtual HRESULT __stdcall AddLine (
        /*[in]*/ float BeginX,
        /*[in]*/ float BeginY,
        /*[in]*/ float EndX,
        /*[in]*/ float EndY,
        /*[out,retval]*/ struct Shape * * Line ) = 0;
      virtual HRESULT __stdcall AddPicture (
        /*[in]*/ BSTR FileName,
        /*[in]*/ enum Office2010::MsoTriState LinkToFile,
        /*[in]*/ enum Office2010::MsoTriState SaveWithDocument,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * Picture ) = 0;
      virtual HRESULT __stdcall AddPolyline (
        /*[in]*/ VARIANT SafeArrayOfPoints,
        /*[out,retval]*/ struct Shape * * Polyline ) = 0;
      virtual HRESULT __stdcall AddShape (
        /*[in]*/ enum Office2010::MsoAutoShapeType Type,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * Shape ) = 0;
      virtual HRESULT __stdcall AddTextEffect (
        /*[in]*/ enum Office2010::MsoPresetTextEffect PresetTextEffect,
        /*[in]*/ BSTR Text,
        /*[in]*/ BSTR FontName,
        /*[in]*/ float FontSize,
        /*[in]*/ enum Office2010::MsoTriState FontBold,
        /*[in]*/ enum Office2010::MsoTriState FontItalic,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[out,retval]*/ struct Shape * * TextEffect ) = 0;
      virtual HRESULT __stdcall AddTextbox (
        /*[in]*/ enum Office2010::MsoTextOrientation Orientation,
        /*[in]*/ float Left,
        /*[in]*/ float Top,
        /*[in]*/ float Width,
        /*[in]*/ float Height,
        /*[out,retval]*/ struct Shape * * Textbox ) = 0;
      virtual HRESULT __stdcall BuildFreeform (
        /*[in]*/ enum Office2010::MsoEditingType EditingType,
        /*[in]*/ float X1,
        /*[in]*/ float Y1,
        /*[out,retval]*/ struct FreeformBuilder * * FreeformBuilder ) = 0;
      virtual HRESULT __stdcall Range (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct ShapeRange * * Range ) = 0;
      virtual HRESULT __stdcall SelectAll ( ) = 0;
      virtual HRESULT __stdcall get_Background (
        /*[out,retval]*/ struct Shape * * Background ) = 0;
};

struct __declspec(uuid("914934f3-5a91-11cf-8700-00aa0060263b"))
CustomLayout : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Shapes (
        /*[out,retval]*/ struct Shapes * * Shapes ) = 0;
      virtual HRESULT __stdcall get_HeadersFooters (
        /*[out,retval]*/ struct HeadersFooters * * HeadersFooters ) = 0;
      virtual HRESULT __stdcall get_Background (
        /*[out,retval]*/ struct ShapeRange * * Background ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall put_Name (
        /*[in]*/ BSTR Name ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
      virtual HRESULT __stdcall get_Height (
        /*[out,retval]*/ float * Height ) = 0;
      virtual HRESULT __stdcall get_Width (
        /*[out,retval]*/ float * Width ) = 0;
      virtual HRESULT __stdcall get_Hyperlinks (
        /*[out,retval]*/ struct Hyperlinks * * Hyperlinks ) = 0;
      virtual HRESULT __stdcall get_Design (
        /*[out,retval]*/ struct Design * * Design ) = 0;
      virtual HRESULT __stdcall get_TimeLine (
        /*[out,retval]*/ struct TimeLine * * TimeLine ) = 0;
      virtual HRESULT __stdcall get_SlideShowTransition (
        /*[out,retval]*/ struct SlideShowTransition * * SlideShowTransition ) = 0;
      virtual HRESULT __stdcall get_MatchingName (
        /*[out,retval]*/ BSTR * MatchingName ) = 0;
      virtual HRESULT __stdcall put_MatchingName (
        /*[in]*/ BSTR MatchingName ) = 0;
      virtual HRESULT __stdcall get_Preserved (
        /*[out,retval]*/ enum Office2010::MsoTriState * Preserved ) = 0;
      virtual HRESULT __stdcall put_Preserved (
        /*[in]*/ enum Office2010::MsoTriState Preserved ) = 0;
      virtual HRESULT __stdcall get_Index (
        /*[out,retval]*/ int * Index ) = 0;
      virtual HRESULT __stdcall Select ( ) = 0;
      virtual HRESULT __stdcall Cut ( ) = 0;
      virtual HRESULT __stdcall Copy ( ) = 0;
      virtual HRESULT __stdcall Duplicate (
        /*[out,retval]*/ struct CustomLayout * * Duplicate ) = 0;
      virtual HRESULT __stdcall MoveTo (
        /*[in]*/ int toPos ) = 0;
      virtual HRESULT __stdcall get_DisplayMasterShapes (
        /*[out,retval]*/ enum Office2010::MsoTriState * DisplayMasterShapes ) = 0;
      virtual HRESULT __stdcall put_DisplayMasterShapes (
        /*[in]*/ enum Office2010::MsoTriState DisplayMasterShapes ) = 0;
      virtual HRESULT __stdcall get_FollowMasterBackground (
        /*[out,retval]*/ enum Office2010::MsoTriState * FollowMasterBackground ) = 0;
      virtual HRESULT __stdcall put_FollowMasterBackground (
        /*[in]*/ enum Office2010::MsoTriState FollowMasterBackground ) = 0;
      virtual HRESULT __stdcall get_ThemeColorScheme (
        /*[out,retval]*/ struct Office2010::ThemeColorScheme * * ThemeColorScheme ) = 0;
      virtual HRESULT __stdcall get_CustomerData (
        /*[out,retval]*/ struct CustomerData * * CustomerData ) = 0;
};

struct __declspec(uuid("914934f2-5a91-11cf-8700-00aa0060263b"))
CustomLayouts : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct CustomLayout * * Item ) = 0;
      virtual HRESULT __stdcall Add (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct CustomLayout * * Add ) = 0;
      virtual HRESULT __stdcall Paste (
        /*[in]*/ int Index,
        /*[out,retval]*/ struct CustomLayout * * Paste ) = 0;
};

struct __declspec(uuid("914934d8-5a91-11cf-8700-00aa0060263b"))
DiagramNode : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall AddNode (
        /*[in]*/ enum Office2010::MsoRelativeNodePosition Pos,
        /*[in]*/ enum Office2010::MsoDiagramNodeType NodeType,
        /*[out,retval]*/ struct DiagramNode * * NewNode ) = 0;
      virtual HRESULT __stdcall Delete ( ) = 0;
      virtual HRESULT __stdcall MoveNode (
        /*[in]*/ struct DiagramNode * TargetNode,
        /*[in]*/ enum Office2010::MsoRelativeNodePosition Pos ) = 0;
      virtual HRESULT __stdcall ReplaceNode (
        /*[in]*/ struct DiagramNode * TargetNode ) = 0;
      virtual HRESULT __stdcall SwapNode (
        /*[in]*/ struct DiagramNode * TargetNode,
        /*[in]*/ VARIANT_BOOL SwapChildren ) = 0;
      virtual HRESULT __stdcall CloneNode (
        /*[in]*/ VARIANT_BOOL CopyChildren,
        /*[in]*/ struct DiagramNode * TargetNode,
        /*[in]*/ enum Office2010::MsoRelativeNodePosition Pos,
        /*[out,retval]*/ struct DiagramNode * * Node ) = 0;
      virtual HRESULT __stdcall TransferChildren (
        /*[in]*/ struct DiagramNode * ReceivingNode ) = 0;
      virtual HRESULT __stdcall NextNode (
        /*[out,retval]*/ struct DiagramNode * * NextNode ) = 0;
      virtual HRESULT __stdcall PrevNode (
        /*[out,retval]*/ struct DiagramNode * * PrevNode ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Children (
        /*[out,retval]*/ struct DiagramNodeChildren * * Children ) = 0;
      virtual HRESULT __stdcall get_Shape (
        /*[out,retval]*/ struct Shape * * Shape ) = 0;
      virtual HRESULT __stdcall get_Root (
        /*[out,retval]*/ struct DiagramNode * * Root ) = 0;
      virtual HRESULT __stdcall get_Diagram (
        /*[out,retval]*/ struct Diagram * * Diagram ) = 0;
      virtual HRESULT __stdcall get_Layout (
        /*[out,retval]*/ enum Office2010::MsoOrgChartLayoutType * Type ) = 0;
      virtual HRESULT __stdcall put_Layout (
        /*[in]*/ enum Office2010::MsoOrgChartLayoutType Type ) = 0;
      virtual HRESULT __stdcall get_TextShape (
        /*[out,retval]*/ struct Shape * * Shape ) = 0;
};

struct __declspec(uuid("914934d9-5a91-11cf-8700-00aa0060263b"))
DiagramNodeChildren : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get__NewEnum (
        /*[out,retval]*/ IUnknown * * ppunkEnum ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct DiagramNode * * Node ) = 0;
      virtual HRESULT __stdcall AddNode (
        /*[in]*/ VARIANT Index,
        /*[in]*/ enum Office2010::MsoDiagramNodeType NodeType,
        /*[out,retval]*/ struct DiagramNode * * NewNode ) = 0;
      virtual HRESULT __stdcall SelectAll ( ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ int * iDiagramNodes ) = 0;
      virtual HRESULT __stdcall get_FirstChild (
        /*[out,retval]*/ struct DiagramNode * * First ) = 0;
      virtual HRESULT __stdcall get_LastChild (
        /*[out,retval]*/ struct DiagramNode * * Last ) = 0;
};

struct __declspec(uuid("914934da-5a91-11cf-8700-00aa0060263b"))
DiagramNodes : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get__NewEnum (
        /*[out,retval]*/ IUnknown * * ppunkEnum ) = 0;
      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct DiagramNode * * ppdn ) = 0;
      virtual HRESULT __stdcall SelectAll ( ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ int * iDiagramNodes ) = 0;
};

struct __declspec(uuid("914934db-5a91-11cf-8700-00aa0060263b"))
Diagram : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ IDispatch * * ppidisp ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ long * plCreator ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ IDispatch * * Parent ) = 0;
      virtual HRESULT __stdcall get_Nodes (
        /*[out,retval]*/ struct DiagramNodes * * Nodes ) = 0;
      virtual HRESULT __stdcall get_Type (
        /*[out,retval]*/ enum Office2010::MsoDiagramType * Type ) = 0;
      virtual HRESULT __stdcall get_AutoLayout (
        /*[out,retval]*/ enum Office2010::MsoTriState * AutoLayout ) = 0;
      virtual HRESULT __stdcall put_AutoLayout (
        /*[in]*/ enum Office2010::MsoTriState AutoLayout ) = 0;
      virtual HRESULT __stdcall get_Reverse (
        /*[out,retval]*/ enum Office2010::MsoTriState * Reverse ) = 0;
      virtual HRESULT __stdcall put_Reverse (
        /*[in]*/ enum Office2010::MsoTriState Reverse ) = 0;
      virtual HRESULT __stdcall get_AutoFormat (
        /*[out,retval]*/ enum Office2010::MsoTriState * AutoFormat ) = 0;
      virtual HRESULT __stdcall put_AutoFormat (
        /*[in]*/ enum Office2010::MsoTriState AutoFormat ) = 0;
      virtual HRESULT __stdcall Convert (
        /*[in]*/ enum Office2010::MsoDiagramType Type ) = 0;
      virtual HRESULT __stdcall FitText ( ) = 0;
};

struct __declspec(uuid("92d41a50-f07e-4ca4-af6f-bef486aa4e6f"))
FileConverters : Collection
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall Item (
        /*[in]*/ VARIANT Index,
        /*[out,retval]*/ struct FileConverter * * Item ) = 0;
};

struct __declspec(uuid("92d41a51-f07e-4ca4-af6f-bef486aa4e6f"))
FileConverter : IDispatch
{
    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Application (
        /*[out,retval]*/ struct _Application * * Application ) = 0;
      virtual HRESULT __stdcall get_CanOpen (
        /*[out,retval]*/ VARIANT_BOOL * CanOpen ) = 0;
      virtual HRESULT __stdcall get_CanSave (
        /*[out,retval]*/ VARIANT_BOOL * CanSave ) = 0;
      virtual HRESULT __stdcall get_ClassName (
        /*[out,retval]*/ BSTR * ClassName ) = 0;
      virtual HRESULT __stdcall get_Creator (
        /*[out,retval]*/ struct FileConverters * * Creator ) = 0;
      virtual HRESULT __stdcall get_Extensions (
        /*[out,retval]*/ BSTR * Extensions ) = 0;
      virtual HRESULT __stdcall get_FormatName (
        /*[out,retval]*/ BSTR * FormatName ) = 0;
      virtual HRESULT __stdcall get_Name (
        /*[out,retval]*/ BSTR * Name ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ struct FileConverters * * Parent ) = 0;
      virtual HRESULT __stdcall get_Path (
        /*[out,retval]*/ BSTR * Path ) = 0;
      virtual HRESULT __stdcall get_OpenFormat (
        /*[out,retval]*/ long * OpenFormat ) = 0;
      virtual HRESULT __stdcall get_SaveFormat (
        /*[out,retval]*/ long * SaveFormat ) = 0;
};

} // namespace PowerPoint2010

#pragma pack(pop)
