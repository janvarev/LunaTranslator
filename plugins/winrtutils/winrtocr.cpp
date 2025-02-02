#include"pch.h"
#include"define.h"
#include<windows.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Media.FaceAnalysis.h>
#include <winrt/Windows.Media.Ocr.h>

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Media.Devices.h>

#include <winrt/Windows.Security.Cryptography.h>
#include <winrt/Windows.Globalization.h>
#include <iostream>
#include <fstream>
#include<vector>
using namespace winrt;

using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Media::Ocr;

using namespace Windows::Devices::Enumeration;
using namespace Windows::Media::Devices;

using namespace Windows::Security::Cryptography;
using namespace Windows::Globalization;
using namespace Windows::Foundation::Collections;
bool check_language_valid(wchar_t* language) {
    OcrEngine ocrEngine = OcrEngine::TryCreateFromUserProfileLanguages();
    std::wstring l = language;
    try {
        Language language1(l);
        return ocrEngine.IsLanguageSupported(language1);
    }
    catch (...) {
        return false;
    }
}
wchar_t** getlanguagelist(int* num) {
    OcrEngine ocrEngine = OcrEngine::TryCreateFromUserProfileLanguages();
    auto languages = ocrEngine.AvailableRecognizerLanguages();
    auto ret = new wchar_t* [languages.Size()];
    int i = 0;
    for (auto&& language : languages)
    {
        //std::wcout << language.LanguageTag().c_str() << L" " << language.DisplayName().c_str() << L" " << language.AbbreviatedName().c_str() << L'\n';
        //zh-Hans-CN  中文(简体，中国)  简体
        //ja  日语   
        auto lang = language.LanguageTag();
        size_t len = lang.size() + 1;
        ret[i] = new wchar_t[len];
        wcscpy_s(ret[i], len, lang.c_str());
        i += 1;
    }
    *num = languages.Size();
    return ret;
}
ocrres OCR(wchar_t* fname, wchar_t* lang, wchar_t* space, int* num)
{
    // 指定要识别的图像文件路径
    std::wstring imagePath = fname;

    // 打开图像文件
    StorageFile imageFile = StorageFile::GetFileFromPathAsync(imagePath).get();
    IRandomAccessStream imageStream = imageFile.OpenAsync(FileAccessMode::Read).get();
    // 创建 BitmapDecoder 对象解码图像
    BitmapDecoder decoder = BitmapDecoder::CreateAsync(imageStream).get();

    // 从解码器中获取位图数据
    SoftwareBitmap softwareBitmap = decoder.GetSoftwareBitmapAsync().get();
    std::wstring l = lang;
    Language language(l);
    // 创建 OcrEngine 对象
    OcrEngine ocrEngine = OcrEngine::TryCreateFromLanguage(language);
    // 创建 OcrResult 对象并进行识别
    OcrResult ocrResult = ocrEngine.RecognizeAsync(softwareBitmap).get();
    // 输出识别结果
    auto res = ocrResult.Lines(); 
    std::vector<std::wstring>rets;
    std::vector< int>ys; 
    int i = 0;
    std::wstring sspace = space;//默认即使日文也有空格
    for (auto line : res)
    {

        std::wstring xx = L"";
        bool start = true;
        float y = 0;
        for (auto word : line.Words()) {
            if (!start)xx += sspace;
            start = false;
            xx += word.Text();
            y = word.BoundingRect().Y;
        }
        ys.push_back(y);
        rets.emplace_back(xx); 
        i += 1;
    }
    *num = res.Size();
    return ocrres{ vecwstr2c(rets),vecint2c(ys)};
}
