#pragma once

#include <cstdint>

namespace Emergence::InputStorage
{
/// \brief Code of the key, localized to user keyboard layout.
using KeyCode = std::int32_t;

/// \brief Physical layout-independent code of the key.
using ScanCode = std::int32_t;

/// \brief Represents which qualifier keys are active.
using QualifiersMask = std::uint32_t;

/// \brief Represents state of the key: whether it is up or down.
enum class KeyState : std::uint8_t
{
    DOWN = 0u,
    UP
};

/// \brief Contains key codes for all supported keys on keyboard. Must be initialized by IO backend.
struct KeyCodeMapping final
{
    KeyCode keyReturn;
    KeyCode keyEscape;
    KeyCode keyBackspace;
    KeyCode keyTab;
    KeyCode keyExclaim;
    KeyCode keyQuoteDouble;
    KeyCode keySpace;
    KeyCode keyHash;
    KeyCode keyPercent;
    KeyCode keyDollar;
    KeyCode keyAmpersand;
    KeyCode keyQuote;
    KeyCode keyLeftParenthesis;
    KeyCode keyRightParenthesis;
    KeyCode keyAsterisk;
    KeyCode keyPlus;
    KeyCode keyComma;
    KeyCode keyMinus;
    KeyCode keyPeriod;
    KeyCode keySlash;
    KeyCode key0;
    KeyCode key1;
    KeyCode key2;
    KeyCode key3;
    KeyCode key4;
    KeyCode key5;
    KeyCode key6;
    KeyCode key7;
    KeyCode key8;
    KeyCode key9;
    KeyCode keyColon;
    KeyCode keySemicolon;
    KeyCode keyLess;
    KeyCode keyEquals;
    KeyCode keyGreater;
    KeyCode keyQuestion;
    KeyCode keyAt;

    KeyCode keyLeftBracket;
    KeyCode keyBackSlash;
    KeyCode keyRightBracket;
    KeyCode keyCaret;
    KeyCode keyUnderscore;
    KeyCode keyBackQuote;
    KeyCode keyA;
    KeyCode keyB;
    KeyCode keyC;
    KeyCode keyD;
    KeyCode keyE;
    KeyCode keyF;
    KeyCode keyG;
    KeyCode keyH;

    // NOLINTNEXTLINE(misc-confusable-identifiers): What can we do, they are keys after all.
    KeyCode keyI;

    KeyCode keyJ;
    KeyCode keyK;
    KeyCode keyL;
    KeyCode keyM;
    KeyCode keyN;

    // NOLINTNEXTLINE(misc-confusable-identifiers): What can we do, they are keys after all.
    KeyCode keyO;

    KeyCode keyP;
    KeyCode keyQ;
    KeyCode keyR;
    KeyCode keyS;
    KeyCode keyT;
    KeyCode keyU;
    KeyCode keyV;
    KeyCode keyW;
    KeyCode keyX;
    KeyCode keyY;
    KeyCode keyZ;

    KeyCode keyCapsLock;

    KeyCode keyF1;
    KeyCode keyF2;
    KeyCode keyF3;
    KeyCode keyF4;
    KeyCode keyF5;
    KeyCode keyF6;
    KeyCode keyF7;
    KeyCode keyF8;
    KeyCode keyF9;
    KeyCode keyF10;
    KeyCode keyF11;
    KeyCode keyF12;

    KeyCode keyPrintScreen;
    KeyCode keyScrollLock;
    KeyCode keyPause;
    KeyCode keyInsert;
    KeyCode keyHome;
    KeyCode keyPageUp;
    KeyCode keyDelete;
    KeyCode keyEnd;
    KeyCode keyPageDown;
    KeyCode keyRight;
    KeyCode keyLeft;
    KeyCode keyDown;
    KeyCode keyUp;

    KeyCode keyNumLockClear;
    KeyCode keyPadDivide;
    KeyCode keyPadMultiply;
    KeyCode keyPadMinus;
    KeyCode keyPadPlus;
    KeyCode keyPadEnter;
    KeyCode keyPad1;
    KeyCode keyPad2;
    KeyCode keyPad3;
    KeyCode keyPad4;
    KeyCode keyPad5;
    KeyCode keyPad6;
    KeyCode keyPad7;
    KeyCode keyPad8;
    KeyCode keyPad9;
    KeyCode keyPad0;
    KeyCode keyPadPeriod;

    KeyCode keyApplication;
    KeyCode keyPower;
    KeyCode keyPadEquals;
    KeyCode keyPadF13;
    KeyCode keyPadF14;
    KeyCode keyPadF15;
    KeyCode keyPadF16;
    KeyCode keyPadF17;
    KeyCode keyPadF18;
    KeyCode keyPadF19;
    KeyCode keyPadF20;
    KeyCode keyPadF21;
    KeyCode keyPadF22;
    KeyCode keyPadF23;
    KeyCode keyPadF24;
    KeyCode keyExecute;
    KeyCode keyHelp;
    KeyCode keyMenu;
    KeyCode keySelect;
    KeyCode keyStop;
    KeyCode keyAgain;
    KeyCode keyUndo;
    KeyCode keyCut;
    KeyCode keyCopy;
    KeyCode keyPaste;
    KeyCode keyFind;
    KeyCode keyMute;
    KeyCode keyVolumeUp;
    KeyCode keyVolumeDown;
    KeyCode keyPadComma;
    KeyCode keyPadEqualsAs400;

    KeyCode keyAltErase;
    KeyCode keySystemRequest;
    KeyCode keyCancel;
    KeyCode keyClear;
    KeyCode keyPrior;
    KeyCode keyReturn2;
    KeyCode keySeparator;
    KeyCode keyOut;
    KeyCode keyOperation;
    KeyCode keyClearAgain;
    KeyCode keyCarousel;
    KeyCode keyExSel;

    KeyCode keyPad00;
    KeyCode keyPad000;
    KeyCode keyThousandsSeparator;
    KeyCode keyDecimalSeparator;
    KeyCode keyCurrencyUnit;
    KeyCode keyCurrencySubUnit;
    KeyCode keyPadLeftParenthesis;
    KeyCode keyPadRightParenthesis;
    KeyCode keyPadLeftBrace;
    KeyCode keyPadRightBrace;
    KeyCode keyPadTab;
    KeyCode keyPadBackspace;
    KeyCode keyPadA;
    KeyCode keyPadB;
    KeyCode keyPadC;
    KeyCode keyPadD;
    KeyCode keyPadE;
    KeyCode keyPadF;
    KeyCode keyPadXOR;
    KeyCode keyPadPower;
    KeyCode keyPadPercent;
    KeyCode keyPadLess;
    KeyCode keyPadGreater;
    KeyCode keyPadAmpersand;
    KeyCode keyPadDoubleAmpersand;
    KeyCode keyPadVerticalBar;
    KeyCode keyPadDoubleVerticalBar;
    KeyCode keyPadColon;
    KeyCode keyPadHash;
    KeyCode keyPadSpace;
    KeyCode keyPadAt;
    KeyCode keyPadExclaim;
    KeyCode keyPadMemStore;
    KeyCode keyPadMemRecall;
    KeyCode keyPadMemClear;
    KeyCode keyPadMemAdd;
    KeyCode keyPadMemSubtract;
    KeyCode keyPadMemMultiply;
    KeyCode keyPadMemDivide;
    KeyCode keyPadPlusMinus;
    KeyCode keyPadClear;
    KeyCode keyPadClearEntry;
    KeyCode keyPadBinary;
    KeyCode keyPadOctal;
    KeyCode keyPadDecimal;
    KeyCode keyPadHexadecimal;

    KeyCode keyLeftControl;
    KeyCode keyLeftShift;
    KeyCode keyLeftAlt;
    KeyCode keyLeftGui;
    KeyCode keyRightControl;
    KeyCode keyRightShift;
    KeyCode keyRightAlt;
    KeyCode keyRightGui;

    KeyCode keyMode;

    KeyCode keyAudioNext;
    KeyCode keyAudioPrev;
    KeyCode keyAudioStop;
    KeyCode keyAudioPlay;
    KeyCode keyAudioMute;
    KeyCode keyMediaSelect;
    KeyCode keyWww;
    KeyCode keyMail;
    KeyCode keyCalculator;
    KeyCode keyComputer;
    KeyCode keyApplicationControlSearch;
    KeyCode keyApplicationControlHome;
    KeyCode keyApplicationControlBack;
    KeyCode keyApplicationControlForward;
    KeyCode keyApplicationControlStop;
    KeyCode keyApplicationControlRefresh;
    KeyCode keyApplicationControlBookmarks;

    KeyCode keyBrightnessDown;
    KeyCode keyBrightnessUp;
    KeyCode keyDisplaySwitch;
    KeyCode keyKeyboardIlluminationToggle;
    KeyCode keyKeyboardIlluminationDown;
    KeyCode keyKeyboardIlluminationUp;
    KeyCode keyEject;
    KeyCode keySleep;
    KeyCode keyApp1;
    KeyCode keyApp2;

    KeyCode keyAudioRewind;
    KeyCode keyAudioFastForward;
};

/// \brief Contains qualifier mask values for all supported qualifiers. Must be initialized by IO backend.
/// \details Qualifier masks are allowed to be freely combined.
struct QualifiersMapping final
{
    QualifiersMask qualifierLeftShift;
    QualifiersMask qualifierRightShift;
    QualifiersMask qualifierLeftControl;
    QualifiersMask qualifierLeftAlt;
    QualifiersMask qualifierRightAlt;
    QualifiersMask qualifierNumPad;
    QualifiersMask qualifierCapsLock;
};
} // namespace Emergence::InputStorage
