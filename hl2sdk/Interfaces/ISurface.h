#pragma once
#include "IAppSystem.h"
#include "IVPanel.h"
#include "IMaterial.h"
#include "IInputSystem.h"
#include "../Structs/color.h"
#include "../../l4d2Simple2/vector.h"

#define FW_DONTCARE         0
#define FW_THIN             100
#define FW_EXTRALIGHT       200
#define FW_LIGHT            300
#define FW_NORMAL           400
#define FW_MEDIUM           500
#define FW_SEMIBOLD         600
#define FW_BOLD             700
#define FW_EXTRABOLD        800
#define FW_HEAVY            900

enum FontFlags_t
{
	FONTFLAG_NONE,
	FONTFLAG_ITALIC = 0x001,
	FONTFLAG_UNDERLINE = 0x002,
	FONTFLAG_STRIKEOUT = 0x004,
	FONTFLAG_SYMBOL = 0x008,
	FONTFLAG_ANTIALIAS = 0x010,
	FONTFLAG_GAUSSIANBLUR = 0x020,
	FONTFLAG_ROTARY = 0x040,
	FONTFLAG_DROPSHADOW = 0x080,
	FONTFLAG_ADDITIVE = 0x100,
	FONTFLAG_OUTLINE = 0x200,
	FONTFLAG_CUSTOM = 0x400,
	FONTFLAG_BITMAP = 0x800,
};

enum FontDrawType_t
{
	// Use the "additive" value from the scheme file
	FONT_DRAW_DEFAULT = 0,

	// Overrides
	FONT_DRAW_NONADDITIVE,
	FONT_DRAW_ADDITIVE,

	FONT_DRAW_TYPE_COUNT = 2,
};

typedef unsigned long HFont;

struct FontVertex_t
{
	Vector2D m_Position;
	Vector2D m_TexCoord;

	inline FontVertex_t() {}

	inline FontVertex_t(const Vector2D &pos, const Vector2D &coord = Vector2D(0, 0))
	{
		m_Position = pos;
		m_TexCoord = coord;
	}
	inline void Init(const Vector2D &pos, const Vector2D &coord = Vector2D(0, 0))
	{
		m_Position = pos;
		m_TexCoord = coord;
	}
};

typedef FontVertex_t Vertex_t;

struct IntRect
{
	int x0;
	int y0;
	int x1;
	int y1;
};

// Refactor these two
struct CharRenderInfo
{
	// Text pos
	int				x, y;
	// Top left and bottom right
	// This is now a pointer to an array maintained by the surface, to avoid copying the data on the 360
	Vertex_t		*verts;
	int				textureId;
	int				abcA;
	int				abcB;
	int				abcC;
	int				fontTall;
	HFont			currentFont;
	// In:
	FontDrawType_t	drawType;
	wchar_t			ch;

	// Out
	bool			valid;
	// In/Out (true by default)
	bool			shouldclip;
};

enum CursorCode
{
	dc_user,
	dc_none,
	dc_arrow,
	dc_ibeam,
	dc_hourglass,
	dc_waitarrow,
	dc_crosshair,
	dc_up,
	dc_sizenwse,
	dc_sizenesw,
	dc_sizewe,
	dc_sizens,
	dc_sizeall,
	dc_no,
	dc_hand,
	dc_blank, // don't show any custom vgui cursor, just let windows do it stuff (for HTML widget)
	dc_last,
	dc_alwaysvisible_push,
	dc_alwaysvisible_pop,
};

class IImage;
typedef ButtonCode_t MouseCode;
typedef ButtonCode_t KeyCode;

//-----------------------------------------------------------------------------
// Purpose: basic interface for a HTML window
//-----------------------------------------------------------------------------
class IHTML
{
public:
	// open a new page
	virtual void OpenURL(const char *) = 0;

	// stops the existing page from loading
	virtual bool StopLoading() = 0;

	// refreshes the current page
	virtual bool Refresh() = 0;

	// display the control -- deprecated !! Use SetVisible() instead!
	virtual bool Show(bool shown) = 0;

	// return the currently opened page
	virtual const char *GetOpenedPage() = 0;

	// called when the browser needs to be resized
	virtual void Obsolete_OnSize(int x, int y, int w, int h) = 0;

	// returns the width and height (in pixels) of the HTML full page (not just the displayed region)
	virtual void GetHTMLSize(int &wide, int &tall) = 0;


	// clear the text in an existing control
	virtual void Clear() = 0;

	// add text to the browser control (as a HTML formated string)
	virtual void AddText(const char *text) = 0;

	enum MOUSE_STATE { UP, DOWN, MOVE, DBLCLICK };
	// unused functions we keep around so the vtable layout is binary compatible
	virtual void Obsolete_OnMouse(MouseCode code, MOUSE_STATE s, int x, int y) = 0;
	virtual void Obsolete_OnChar(wchar_t unichar) = 0;
	virtual void Obsolete_OnKeyDown(KeyCode code) = 0;

	virtual IImage *GetBitmap() = 0;
	virtual void SetVisible(bool state) = 0;


	virtual void SetSize(int wide, int tall) = 0;

	virtual void OnMouse(MouseCode code, MOUSE_STATE s, int x, int y, bool bPopupMenuMenu) = 0;
	virtual void OnChar(wchar_t unichar, bool bPopupMenu) = 0;
	virtual void OnKeyDown(KeyCode code, bool bPopupMenu) = 0;

	virtual void ScrollV(int nPixels) = 0;
	virtual void ScrollH(int nPixels) = 0;
	virtual void OnMouseWheeled(int delta, bool bPopupMenu) = 0;

	// called when the browser needs to be resized
	virtual void OnKeyUp(KeyCode code, bool bPopupMenu) = 0;


	// open a URL with the provided POST data (which can be much larger than the max URL of 512 chars)
	// NOTE - You CANNOT have get params (i.e a "?" ) in pchURL if pchPostData is set (due to an IE bug)
	virtual void PostURL(const char *pchURL, const char *pchPostData) = 0;

	// Run javascript within the browser control
	virtual void RunJavascript(const char *pchScript) = 0;

	virtual void SetMousePosition(int x, int y, bool bPopupMenu) = 0;

	virtual void SetUserAgentInfo(const wchar_t *pwchUserAgent) = 0;

	// can't add custom headers to IE
	virtual void AddHeader(const char *pchHeader, const char *pchValue) = 0;

	virtual void SetFileDialogChoice(const char *pchFileName) = 0;

	// we are hiding the popup, so make sure webkit knows
	virtual void HidePopup() = 0;
	virtual void SetHTMLFocus() = 0;
	virtual void KillHTMLFocus() = 0;
	// ask webkit about the size of any scrollbars it wants to render
	virtual void HorizontalScrollBarSize(int &x, int &y, int &wide, int &tall) = 0;
	virtual void VerticalScrollBarSize(int &x, int &y, int &wide, int &tall) = 0;
	virtual int HorizontalScroll() = 0;
	virtual int VerticalScroll() = 0;
	virtual int HorizontalScrollMax() = 0;
	virtual int VerticalScrollMax() = 0;
	virtual bool IsHorizontalScrollBarVisible() = 0;
	virtual bool IsVeritcalScrollBarVisible() = 0;
	virtual void SetHorizontalScroll(int scroll) = 0;
	virtual void SetVerticalScroll(int scroll) = 0;
	virtual void ViewSource() = 0;
	virtual void Copy() = 0;
	virtual void Paste() = 0;

	// IE specific calls
	virtual bool IsIERender() = 0;
	virtual void GetIDispatchPtr(void **pIDispatch) = 0;
	virtual void GetHTMLScroll(int &top, int &left) = 0;
};


//-----------------------------------------------------------------------------
// Purpose: possible load errors when you open a url in the web browser
//-----------------------------------------------------------------------------
enum EWebPageLoadError
{
	eLoadErrorNone = 0,
	eMimeTypeNotSupported, // probably trying to download an exe or something
	eCacheMiss, // Usually caused by navigating to a page with POST data via back or forward buttons
	eBadURL, // bad url passed in (invalid hostname, malformed)
	eConnectionProblem, // network connectivity problem, server offline or user not on internet
	eProxyConnectionProblem, // User is configured to use proxy, but we can't use it

	eLoadErrorUnknown, // not a load type we classify right now, check out cef_handler_errorcode_t for the full list we could translate
};

typedef unsigned long HTexture;

//-----------------------------------------------------------------------------
// Purpose: Interface to drawing an image
//-----------------------------------------------------------------------------
class IImage
{
public:
	// Call to Paint the image
	// Image will draw within the current panel context at the specified position
	virtual void Paint() = 0;

	// Set the position of the image
	virtual void SetPos(int x, int y) = 0;

	// Gets the size of the content
	virtual void GetContentSize(int &wide, int &tall) = 0;

	// Get the size the image will actually draw in (usually defaults to the content size)
	virtual void GetSize(int &wide, int &tall) = 0;

	// Sets the size of the image
	virtual void SetSize(int wide, int tall) = 0;

	// Set the draw color 
	virtual void SetColor(Color col) = 0;

	// virtual destructor
	virtual ~IImage() {}

	// not for general purpose use
	// evicts the underlying image from memory if refcounts permit, otherwise ignored
	// returns true if eviction occurred, otherwise false
	virtual bool Evict() = 0;

	virtual int GetNumFrames() = 0;
	virtual void SetFrame(int nFrame) = 0;
	virtual HTexture GetID() = 0;

	virtual void SetRotation(int iRotation) = 0;
};

//-----------------------------------------------------------------------------
// Purpose: basic callback interface for a HTML window
//-----------------------------------------------------------------------------
class IHTMLEvents
{
public:
	// unused functions we keep around so the vtable layout is binary compatible
	virtual bool Obsolete_OnStartURL(const char *url, const char *target, bool first) = 0;
	virtual void Obsolete_OnFinishURL(const char *url) = 0;
	virtual void Obsolete_OnProgressURL(long current, long maximum) = 0;
	virtual void Obsolete_OnSetStatusText(const char *text) = 0;
	virtual void Obsolete_OnUpdate() = 0;
	virtual void Obsolete_OnLink() = 0;
	virtual void Obsolete_OffLink() = 0;

	// call backs for events
	// when the top level browser is changing the page they are looking at (not sub iframes or the like loading)
	virtual void OnURLChanged(const char *url, const char *pchPostData, bool bIsRedirect) = 0;
	// the control has finished loading a request, could be a sub request in the page
	virtual void OnFinishRequest(const char *url, const char *pageTitle) = 0;

	// the lower html control wants to load a url, do we allow it?
	virtual bool OnStartRequestInternal(const char *url, const char *target, const char *pchPostData, bool bIsRedirect) = 0;

	// show a popup menu for this html control
	virtual void ShowPopup(int x, int y, int wide, int tall) = 0;
	// hide any popup menu you are showing
	virtual void HidePopup() = 0;
	// show an external html window at this position and side
	virtual bool OnPopupHTMLWindow(const char *pchURL, int x, int y, int wide, int tall) = 0;
	// the browser is telling us the title it would like us to show
	virtual void SetHTMLTitle(const char *pchTitle) = 0;
	// the browser is loading a sub url for a page, usually an image or css
	virtual void OnLoadingResource(const char *pchURL) = 0;
	// the browser is telling us the user is hovering a url or the like 
	virtual void OnSetStatusText(const char *text) = 0;
	// the browser wants the cursor changed please
	virtual void OnSetCursor(CursorCode cursor) = 0;
	// the browser wants to ask the user to select a local file and tell it about it
	virtual void OnFileLoadDialog(const char *pchTitle, const char *pchInitialFile) = 0;
	// show and hide tooltip text
	virtual void OnShowToolTip(const char *pchText) = 0;
	virtual void OnUpdateToolTip(const char *pchText) = 0;
	virtual void OnHideToolTip() = 0;


	// IE only code
	virtual bool BOnCreateNewWindow(void **ppDispatch) = 0;
	virtual void OnLink() = 0;
	virtual void OffLink() = 0;
	virtual void OnCloseWindow() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnProgressRequest(long current, long maximum) = 0;

	// new Chrome calls
	virtual bool OnOpenNewTab(const char *pchURL, bool bForeground) = 0;
};

// wrapper for IMaterialVar
class IVguiMatInfoVar
{
public:
	// Add a virtual destructor to silence the clang warning.
	// This is harmless but not important since the only derived class
	// doesn't have a destructor.
	virtual ~IVguiMatInfoVar() {}

	virtual int GetIntValue(void) const = 0;
	virtual void SetIntValue(int val) = 0;

	// todo: if you need to add more IMaterialVar functions add them here
};

// wrapper for IMaterial
class IVguiMatInfo
{
public:
	// Add a virtual destructor to silence the clang warning.
	// This is harmless but not important since the only derived class
	// doesn't have a destructor.
	virtual ~IVguiMatInfo() {}

	// make sure to delete the returned object after use!
	virtual IVguiMatInfoVar* FindVarFactory(const char *varName, bool *found) = 0;

	virtual int GetNumAnimationFrames() = 0;

	// todo: if you need to add more IMaterial functions add them here
};

class ISurface : public IAppSystem
{
public:
#ifdef _CSGO
	void		DrawSetColor(int r, int g, int b, int a);
	void		DrawSetColor(Color col);
	void		DrawFilledRect(int x0, int y0, int x1, int y1);
	void		DrawOutlinedRect(int x0, int y0, int x1, int y1);
	void		DrawLine(int x0, int y0, int x1, int y1);
	void		DrawPolyLine(int *px, int *py, int numPoints);
	void		DrawSetTextFont(HFont font);
	void		DrawSetTextColor(int r, int g, int b, int a);
	void		DrawSetTextColor(Color col);
	void		DrawSetTextPos(int x, int y);
	void		DrawPrintText(const wchar_t *text, int textLen, FontDrawType_t drawType = FONT_DRAW_DEFAULT);
	void		DrawSetTextureRGBA(int id, const unsigned char *rgba, int wide, int tall);
	void		DrawSetTexture(int id);
	int			CreateNewTextureID(bool procedural = false);
	HFont		Create_Font();
	bool		SetFontGlyphSet(HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int nRangeMin = 0, int nRangeMax = 0);
	void		GetTextSize(HFont font, const wchar_t *text, int &wide, int &tall);
	void		DrawOutlinedCircle(int x, int y, int radius, int segments);
	void		DrawTexturedPolygon(int n, Vertex_t *pVertice, bool bClipVertices = true);
#else
	// call to Shutdown surface; surface can no longer be used after this is called
	virtual void Shutdown() = 0;

	// frame
	virtual void RunFrame() = 0;

	// hierarchy root
	virtual VPANEL GetEmbeddedPanel() = 0;
	virtual void SetEmbeddedPanel(VPANEL pPanel) = 0;

	// drawing context
	virtual void PushMakeCurrent(VPANEL panel, bool useInsets) = 0;
	virtual void PopMakeCurrent(VPANEL panel) = 0;

	// rendering functions
	virtual void DrawSetColor(int r, int g, int b, int a) = 0;
	virtual void DrawSetColor(Color col) = 0;

	virtual void DrawFilledRect(int x0, int y0, int x1, int y1) = 0;
	virtual void DrawFilledRectArray(IntRect *pRects, int numRects) = 0;
	virtual void DrawOutlinedRect(int x0, int y0, int x1, int y1) = 0;

	virtual void DrawLine(int x0, int y0, int x1, int y1) = 0;
	virtual void DrawPolyLine(int *px, int *py, int numPoints) = 0;

	virtual void DrawSetTextFont(HFont font) = 0;
	virtual void DrawSetTextColor(int r, int g, int b, int a) = 0;
	virtual void DrawSetTextColor(Color col) = 0;
	virtual void DrawSetTextPos(int x, int y) = 0;
	virtual void DrawGetTextPos(int& x, int& y) = 0;
	virtual void DrawPrintText(const wchar_t *text, int textLen, FontDrawType_t drawType = FONT_DRAW_DEFAULT) = 0;
	virtual void DrawUnicodeChar(wchar_t wch, FontDrawType_t drawType = FONT_DRAW_DEFAULT) = 0;

	virtual void DrawFlushText() = 0;		// flushes any buffered text (for rendering optimizations)
	virtual IHTML *CreateHTMLWindow(IHTMLEvents *events, VPANEL context) = 0;
	virtual void PaintHTMLWindow(IHTML *htmlwin) = 0;
	virtual void DeleteHTMLWindow(IHTML *htmlwin) = 0;

	enum ETextureFormat
	{
		eTextureFormat_RGBA,
		eTextureFormat_BGRA,
		eTextureFormat_BGRA_Opaque, // bgra format but alpha is always 255, CEF does this, we can use this fact for better perf on win32 gdi
	};

	virtual int	 DrawGetTextureId(char const *filename) = 0;
	virtual bool DrawGetTextureFile(int id, char *filename, int maxlen) = 0;
	virtual void DrawSetTextureFile(int id, const char *filename, int hardwareFilter, bool forceReload) = 0;
	virtual void DrawSetTextureRGBA(int id, const unsigned char *rgba, int wide, int tall, int hardwareFilter, bool forceReload) = 0;
	virtual void DrawSetTexture(int id) = 0;
	virtual void DrawGetTextureSize(int id, int &wide, int &tall) = 0;
	virtual void DrawTexturedRect(int x0, int y0, int x1, int y1) = 0;
	virtual bool IsTextureIDValid(int id) = 0;
	virtual bool DeleteTextureByID(int id) = 0;

	virtual int CreateNewTextureID(bool procedural = false) = 0;

	virtual void GetScreenSize(int &wide, int &tall) = 0;
	virtual void SetAsTopMost(VPANEL panel, bool state) = 0;
	virtual void BringToFront(VPANEL panel) = 0;
	virtual void SetForegroundWindow(VPANEL panel) = 0;
	virtual void SetPanelVisible(VPANEL panel, bool state) = 0;
	virtual void SetMinimized(VPANEL panel, bool state) = 0;
	virtual bool IsMinimized(VPANEL panel) = 0;
	virtual void FlashWindow(VPANEL panel, bool state) = 0;
	virtual void SetTitle(VPANEL panel, const wchar_t *title) = 0;
	virtual void SetAsToolBar(VPANEL panel, bool state) = 0;		// removes the window's task bar entry (for context menu's, etc.)

																	// windows stuff
	virtual void CreatePopup(VPANEL panel, bool minimised, bool showTaskbarIcon = true, bool disabled = false, bool mouseInput = true, bool kbInput = true) = 0;
	virtual void SwapBuffers(VPANEL panel) = 0;
	virtual void Invalidate(VPANEL panel) = 0;
	virtual void SetCursor(HCursor cursor) = 0;
	virtual void SetCursorAlwaysVisible(bool visible) = 0;
	virtual bool IsCursorVisible() = 0;
	virtual void ApplyChanges() = 0;
	virtual bool IsWithin(int x, int y) = 0;
	virtual bool HasFocus() = 0;

	// returns true if the surface supports minimize & maximize capabilities
	enum SurfaceFeature_e
	{
		ANTIALIASED_FONTS = 1,
		DROPSHADOW_FONTS = 2,
		ESCAPE_KEY = 3,
		OPENING_NEW_HTML_WINDOWS = 4,
		FRAME_MINIMIZE_MAXIMIZE = 5,
		OUTLINE_FONTS = 6,
		DIRECT_HWND_RENDER = 7,
	};
	virtual bool SupportsFeature(SurfaceFeature_e feature) = 0;

	// restricts what gets drawn to one panel and it's children
	// currently only works in the game
	virtual void RestrictPaintToSinglePanel(VPANEL panel) = 0;

	// these two functions obselete, use IInput::SetAppModalSurface() instead
	virtual void SetModalPanel(VPANEL) = 0;
	virtual VPANEL GetModalPanel() = 0;

	virtual void UnlockCursor() = 0;
	virtual void LockCursor() = 0;
	virtual void SetTranslateExtendedKeys(bool state) = 0;
	virtual VPANEL GetTopmostPopup() = 0;

	// engine-only focus handling (replacing WM_FOCUS windows handling)
	virtual void SetTopLevelFocus(VPANEL panel) = 0;

	// fonts
	// creates an empty handle to a vgui font.  windows fonts can be add to this via SetFontGlyphSet().
	virtual HFont CreateFont() = 0;

	// adds to the font
	enum EFontFlags
	{
		FONTFLAG_NONE,
		FONTFLAG_ITALIC = 0x001,
		FONTFLAG_UNDERLINE = 0x002,
		FONTFLAG_STRIKEOUT = 0x004,
		FONTFLAG_SYMBOL = 0x008,
		FONTFLAG_ANTIALIAS = 0x010,
		FONTFLAG_GAUSSIANBLUR = 0x020,
		FONTFLAG_ROTARY = 0x040,
		FONTFLAG_DROPSHADOW = 0x080,
		FONTFLAG_ADDITIVE = 0x100,
		FONTFLAG_OUTLINE = 0x200,
		FONTFLAG_CUSTOM = 0x400,		// custom generated font - never fall back to asian compatibility mode
		FONTFLAG_BITMAP = 0x800,		// compiled bitmap font - no fallbacks
	};

	virtual bool SetFontGlyphSet(HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int nRangeMin = 0, int nRangeMax = 0) = 0;

	// adds a custom font file (only supports true type font files (.ttf) for now)
	virtual bool AddCustomFontFile(const char *fontName, const char *fontFileName) = 0;

	// returns the details about the font
	virtual int GetFontTall(HFont font) = 0;
	virtual int GetFontTallRequested(HFont font) = 0;
	virtual int GetFontAscent(HFont font, wchar_t wch) = 0;
	virtual bool IsFontAdditive(HFont font) = 0;
	virtual void GetCharABCwide(HFont font, int ch, int &a, int &b, int &c) = 0;
	virtual int GetCharacterWidth(HFont font, int ch) = 0;
	virtual void GetTextSize(HFont font, const wchar_t *text, int &wide, int &tall) = 0;

	// notify icons?!?
	virtual VPANEL GetNotifyPanel() = 0;
	virtual void SetNotifyIcon(VPANEL context, HTexture icon, VPANEL panelToReceiveMessages, const char *text) = 0;

	// plays a sound
	virtual void PlaySound(const char *fileName) = 0;

	//!! these functions should not be accessed directly, but only through other vgui items
	//!! need to move these to seperate interface
	virtual int GetPopupCount() = 0;
	virtual VPANEL GetPopup(int index) = 0;
	virtual bool ShouldPaintChildPanel(VPANEL childPanel) = 0;
	virtual bool RecreateContext(VPANEL panel) = 0;
	virtual void AddPanel(VPANEL panel) = 0;
	virtual void ReleasePanel(VPANEL panel) = 0;
	virtual void MovePopupToFront(VPANEL panel) = 0;
	virtual void MovePopupToBack(VPANEL panel) = 0;

	virtual void SolveTraverse(VPANEL panel, bool forceApplySchemeSettings = false) = 0;
	virtual void PaintTraverse(VPANEL panel) = 0;

	virtual void EnableMouseCapture(VPANEL panel, bool state) = 0;

	// returns the size of the workspace
	virtual void GetWorkspaceBounds(int &x, int &y, int &wide, int &tall) = 0;

	// gets the absolute coordinates of the screen (in windows space)
	virtual void GetAbsoluteWindowBounds(int &x, int &y, int &wide, int &tall) = 0;

	// gets the base resolution used in proportional mode
	virtual void GetProportionalBase(int &width, int &height) = 0;

	virtual void CalculateMouseVisible() = 0;
	virtual bool NeedKBInput() = 0;

	virtual bool HasCursorPosFunctions() = 0;
	virtual void SurfaceGetCursorPos(int &x, int &y) = 0;
	virtual void SurfaceSetCursorPos(int x, int y) = 0;

	// SRC only functions!!!
	virtual void DrawTexturedLine(const Vertex_t &a, const Vertex_t &b) = 0;
	virtual void DrawOutlinedCircle(int x, int y, int radius, int segments) = 0;
	virtual void DrawTexturedPolyLine(const Vertex_t *p, int n) = 0; // (Note: this connects the first and last points).
	virtual void DrawTexturedSubRect(int x0, int y0, int x1, int y1, float texs0, float text0, float texs1, float text1) = 0;
	virtual void DrawTexturedPolygon(int n, Vertex_t *pVertice, bool bClipVertices = true) = 0;
	virtual const wchar_t *GetTitle(VPANEL panel) = 0;
	virtual bool IsCursorLocked(void) const = 0;
	virtual void SetWorkspaceInsets(int left, int top, int right, int bottom) = 0;

	// Lower level char drawing code, call DrawGet then pass in info to DrawRender
	virtual bool DrawGetUnicodeCharRenderInfo(wchar_t ch, CharRenderInfo& info) = 0;
	virtual void DrawRenderCharFromInfo(const CharRenderInfo& info) = 0;

	// global alpha setting functions
	// affect all subsequent draw calls - shouldn't normally be used directly, only in Panel::PaintTraverse()
	virtual void DrawSetAlphaMultiplier(float alpha /* [0..1] */) = 0;
	virtual float DrawGetAlphaMultiplier() = 0;

	// web browser
	virtual void SetAllowHTMLJavaScript(bool state) = 0;

	// video mode changing
	virtual void OnScreenSizeChanged(int nOldWidth, int nOldHeight) = 0;

	virtual HCursor CreateCursorFromFile(char const *curOrAniFile, char const *pPathID = 0) = 0;

	// create IVguiMatInfo object ( IMaterial wrapper in VguiMatSurface, NULL in CWin32Surface )
	virtual IVguiMatInfo *DrawGetTextureMatInfoFactory(int id) = 0;

	virtual void PaintTraverseEx(VPANEL panel, bool paintPopups = false) = 0;

	virtual float GetZPos() const = 0;

	// From the Xbox
	virtual void SetPanelForInput(VPANEL vpanel) = 0;
	virtual void DrawFilledRectFastFade(int x0, int y0, int x1, int y1, int fadeStartPt, int fadeEndPt, unsigned int alpha0, unsigned int alpha1, bool bHorizontal) = 0;
	virtual void DrawFilledRectFade(int x0, int y0, int x1, int y1, unsigned int alpha0, unsigned int alpha1, bool bHorizontal) = 0;
	virtual void DrawSetTextureRGBAEx(int id, const unsigned char *rgba, int wide, int tall, ImageFormat imageFormat) = 0;
	virtual void DrawSetTextScale(float sx, float sy) = 0;
	virtual bool SetBitmapFontGlyphSet(HFont font, const char *windowsFontName, float scalex, float scaley, int flags) = 0;
	// adds a bitmap font file
	virtual bool AddBitmapFontFile(const char *fontFileName) = 0;
	// sets a symbol for the bitmap font
	virtual void SetBitmapFontName(const char *pName, const char *pFontFilename) = 0;
	// gets the bitmap font filename
	virtual const char *GetBitmapFontName(const char *pName) = 0;
	virtual void ClearTemporaryFontCache(void) = 0;

	virtual IImage *GetIconImageForFullPath(char const *pFullPath) = 0;
	virtual void DrawUnicodeString(const wchar_t *pwString, FontDrawType_t drawType = FONT_DRAW_DEFAULT) = 0;
	virtual void PrecacheFontCharacters(HFont font, const wchar_t *pCharacters) = 0;
	// Console-only.  Get the string to use for the current video mode for layout files.
	virtual const char *GetResolutionKey(void) const = 0;

	virtual const char *GetFontName(HFont font) = 0;
	virtual const char *GetFontFamilyName(HFont font) = 0;
	virtual void GetKernedCharWidth(HFont font, wchar_t ch, wchar_t chBefore, wchar_t chAfter, float &wide, float &abcA) = 0;

	virtual bool ForceScreenSizeOverride(bool bState, int wide, int tall) = 0;
	// LocalToScreen, ParentLocalToScreen fixups for explicit PaintTraverse calls on Panels not at 0, 0 position
	virtual bool ForceScreenPosOffset(bool bState, int x, int y) = 0;
	virtual void OffsetAbsPos(int &x, int &y) = 0;


	// Causes fonts to get reloaded, etc.
	virtual void ResetFontCaches() = 0;

	virtual int GetTextureNumFrames(int id) = 0;
	virtual void DrawSetTextureFrame(int id, int nFrame, unsigned int *pFrameCache) = 0;
	virtual bool IsScreenSizeOverrideActive(void) = 0;
	virtual bool IsScreenPosOverrideActive(void) = 0;

	virtual void DestroyTextureID(int id) = 0;

	virtual void DrawUpdateRegionTextureRGBA(int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall, ImageFormat imageFormat) = 0;
	virtual bool BHTMLWindowNeedsPaint(IHTML *htmlwin) = 0;

	virtual const char *GetWebkitHTMLUserAgentString() = 0;

	virtual void *Deprecated_AccessChromeHTMLController() = 0;

	// the origin of the viewport on the framebuffer (Which might not be 0,0 for stereo)
	virtual void SetFullscreenViewport(int x, int y, int w, int h) = 0; // this uses NULL for the render target.
	virtual void GetFullscreenViewport(int & x, int & y, int & w, int & h) = 0;
	virtual void PushFullscreenViewport() = 0;
	virtual void PopFullscreenViewport() = 0;

	// handles support for software cursors
	virtual void SetSoftwareCursor(bool bUseSoftwareCursor) = 0;
	virtual void PaintSoftwareCursor() = 0;

#endif
};