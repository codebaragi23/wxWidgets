/////////////////////////////////////////////////////////////////////////////
// Name:        wx/fontutil.h
// Purpose:     font-related helper functions
// Author:      Vadim Zeitlin
// Created:     05.11.99
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// General note: this header is private to wxWidgets and is not supposed to be
// included by user code. The functions declared here are implemented in
// msw/fontutil.cpp for Windows, unix/fontutil.cpp for GTK &c.

#ifndef _WX_FONTUTIL_H_
#define _WX_FONTUTIL_H_

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wx/font.h"        // for wxFont and wxFontEncoding

#if defined(__WXMSW__)
    #include "wx/msw/wrapwin.h"
#endif

#if defined(__WXOSX__)
#include "wx/osx/core/cfref.h"
#endif

class WXDLLIMPEXP_FWD_BASE wxArrayString;
class WXDLLIMPEXP_FWD_CORE wxWindow;
struct WXDLLIMPEXP_FWD_CORE wxNativeEncodingInfo;

// ----------------------------------------------------------------------------
// types
// ----------------------------------------------------------------------------

// wxNativeFontInfo is platform-specific font representation: this struct
// should be considered as opaque font description only used by the native
// functions, the user code can only get the objects of this type from
// somewhere and pass it somewhere else (possibly save them somewhere using
// ToString() and restore them using FromString())

class WXDLLIMPEXP_CORE wxNativeFontInfo
{
public:
#if wxUSE_PANGO
    PangoFontDescription *description;

    // Pango font description doesn't have these attributes, so we store them
    // separately and handle them ourselves in {To,From}String() methods.
    bool m_underlined;
    bool m_strikethrough;
#elif defined(__WXMSW__)
    // Preserve compatibility in the semi-public (i.e. private, but still
    // unfortunately used by some existing code outside of the library) API
    // by allowing to create wxNativeFontInfo from just LOGFONT, but ensure
    // that we always specify the window, to use the correct DPI, when creating
    // fonts inside the library itself.
    wxNativeFontInfo(const LOGFONT& lf_, const wxWindow* win
#ifndef WXBUILDING
        = nullptr
#endif
    );

    // MSW-specific: get point size from LOGFONT height using specified DPI,
    // or screen DPI when 0.
    static double GetPointSizeAtPPI(int lfHeight, int ppi = 0);

    // MSW-specific: get the height value in pixels using LOGFONT convention
    // (i.e. negative) corresponding to the given size in points and DPI.
    static int GetLogFontHeightAtPPI(double size, int ppi);

    LOGFONT      lf;

    // MSW only has limited support for fractional point sizes and we need to
    // store the fractional point size separately if it was initially specified
    // as we can't losslessly recover it from LOGFONT later.
    double       pointSize;
#elif defined(__WXOSX__)
public:
    wxNativeFontInfo(const wxNativeFontInfo& info) { Init(info); }

    ~wxNativeFontInfo() { Free(); }

    wxNativeFontInfo& operator=(const wxNativeFontInfo& info)
    {
        if (this != &info)
        {
            Free();
            Init(info);
        }
        return *this;
    }

    void InitFromFont(CTFontRef font);
    void InitFromFontDescriptor(CTFontDescriptorRef font);
    void Init(const wxNativeFontInfo& info);

    void Free();

    // not all style attributes like condensed etc, are exposed in the public API methods
    // for best fidelity PostScript names are useful, they are also used in the toString/fromString methods
    wxString GetPostScriptName() const;
    bool SetPostScriptName(const wxString& postScriptName);

    static double GetCTWeight( CTFontRef font );
    static double GetCTWeight( CTFontDescriptorRef font );
    static double GetCTwidth( CTFontDescriptorRef font );
    static double GetCTSlant( CTFontDescriptorRef font );

    CTFontDescriptorRef GetCTFontDescriptor() const;

    void RealizeResource() const;
private:
    // attributes for regenerating a CTFontDescriptor, stay close to native values
    // for better roundtrip fidelity
    double        m_ctWeight;
    double        m_ctWidth;
    wxFontStyle   m_style;
    double        m_ctSize;
    wxFontFamily  m_family;

    wxString      m_familyName;
    wxString      m_postScriptName;

    // native font description
    wxCFRef<CTFontDescriptorRef> m_descriptor;
    void          CreateCTFontDescriptor();

    // these attributes are not part of a CTFont
    bool          m_underlined;
    bool          m_strikethrough;
    wxFontEncoding m_encoding;

public :
#elif defined(__WXQT__)
    wxNativeFontInfo();
    wxNativeFontInfo(const wxNativeFontInfo&);
    ~wxNativeFontInfo();
    wxNativeFontInfo& operator=(const wxNativeFontInfo&);

    QFont& m_qtFont;
#else // other platforms
    //
    //  This is a generic implementation that should work on all ports
    //  without specific support by the port.
    //
    #define wxNO_NATIVE_FONTINFO

    double        pointSize;
    wxFontFamily  family;
    wxFontStyle   style;
    int           weight;
    bool          underlined;
    bool          strikethrough;
    wxString      faceName;
    wxFontEncoding encoding;
#endif // platforms

#ifndef __WXQT__
    // default ctor (default copy ctor is ok)
    wxNativeFontInfo() { Init(); }
#endif

#if wxUSE_PANGO
private:
    void Init(const wxNativeFontInfo& info);
    void Free();

public:
    wxNativeFontInfo(const wxNativeFontInfo& info) { Init(info); }
    ~wxNativeFontInfo() { Free(); }

    wxNativeFontInfo& operator=(const wxNativeFontInfo& info)
    {
        if (this != &info)
        {
            Free();
            Init(info);
        }
        return *this;
    }
#endif // wxUSE_PANGO

    // reset to the default state
    void Init();

    // init with the parameters of the given font
    void InitFromFont(const wxFont& font)
    {
#if wxUSE_PANGO || defined(__WXOSX__)
        Init(*font.GetNativeFontInfo());
#else
        // translate all font parameters
        SetStyle((wxFontStyle)font.GetStyle());
        SetNumericWeight(font.GetNumericWeight());
        SetUnderlined(font.GetUnderlined());
        SetStrikethrough(font.GetStrikethrough());
#if defined(__WXMSW__)
        if ( font.IsUsingSizeInPixels() )
            SetPixelSize(font.GetPixelSize());
        else
            SetFractionalPointSize(font.GetFractionalPointSize());
#else
        SetFractionalPointSize(font.GetFractionalPointSize());
#endif

        // set the family/facename
        SetFamily((wxFontFamily)font.GetFamily());
        const wxString& facename = font.GetFaceName();
        if ( !facename.empty() )
        {
            SetFaceName(facename);
        }

        // deal with encoding now (it may override the font family and facename
        // so do it after setting them)
        SetEncoding(font.GetEncoding());
#endif // !wxUSE_PANGO
    }

    // accessors and modifiers for the font elements
    int GetPointSize() const;
    double GetFractionalPointSize() const;
    wxSize GetPixelSize() const;
    wxFontStyle GetStyle() const;
    wxFontWeight GetWeight() const;
    int GetNumericWeight() const;
    bool GetUnderlined() const;
    bool GetStrikethrough() const;
    wxString GetFaceName() const;
    wxFontFamily GetFamily() const;
    wxFontEncoding GetEncoding() const;

    void SetPointSize(int pointsize);
    void SetFractionalPointSize(double pointsize);
    void SetPixelSize(const wxSize& pixelSize);
    void SetStyle(wxFontStyle style);
    void SetNumericWeight(int weight);
    void SetWeight(wxFontWeight weight);
    void SetUnderlined(bool underlined);
    void SetStrikethrough(bool strikethrough);
    bool SetFaceName(const wxString& facename);
    void SetFamily(wxFontFamily family);
    void SetEncoding(wxFontEncoding encoding);

    // Helper used in many ports: use the normal font size if the input is
    // negative, as we handle -1 as meaning this for compatibility.
    void SetSizeOrDefault(double size)
    {
        SetFractionalPointSize
        (
            size < 0 ? wxNORMAL_FONT->GetFractionalPointSize()
                     : size
        );
    }

    // sets the first facename in the given array which is found
    // to be valid. If no valid facename is given, sets the
    // first valid facename returned by wxFontEnumerator::GetFacenames().
    // Does not return a bool since it cannot fail.
    void SetFaceName(const wxArrayString &facenames);


    // it is important to be able to serialize wxNativeFontInfo objects to be
    // able to store them (in config file, for example)
    bool FromString(const wxString& s);
    wxString ToString() const;

    // we also want to present the native font descriptions to the user in some
    // human-readable form (it is not platform independent either, but can
    // hopefully be understood by the user)
    bool FromUserString(const wxString& s);
    wxString ToUserString() const;
};

// ----------------------------------------------------------------------------
// font-related functions (common)
// ----------------------------------------------------------------------------

// translate a wxFontEncoding into native encoding parameter (defined above),
// returning true if an (exact) macth could be found, false otherwise (without
// attempting any substitutions)
WXDLLIMPEXP_CORE bool wxGetNativeFontEncoding(wxFontEncoding encoding,
                                              wxNativeEncodingInfo *info);

// test for the existence of the font described by this facename/encoding,
// return true if such font(s) exist, false otherwise
WXDLLIMPEXP_CORE bool wxTestFontEncoding(const wxNativeEncodingInfo& info);

#endif // _WX_FONTUTIL_H_
