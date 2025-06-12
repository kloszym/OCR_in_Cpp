#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <filesystem>
#include <cmath>

const std::string IMAGE_FOLDER = "../img/";
std::string IMAGE_NAME = "test_image5.png";
std::string tekstGroundTruth = "czcionka cambria zobaczymy czy dziala test test test nie mam juz pomyslu";

struct SzablonObrazu {
    char znak;
    cv::Mat obraz32x32;
};

std::vector<SzablonObrazu> wczytajSzablonyZnakowObrazy(const std::string& folderGlownySzablonow, const std::vector<char>& alfabet);
cv::Mat stworzObrazZPaddingiem(const cv::Mat& obrazWejsciowyBinarny, const cv::Size& docelowyRozmiarFinalny, int padding);
cv::Mat wczytajIPrzetworzWstepnie(const std::string& sciezkaObrazu, bool& sukces);

namespace fs = std::filesystem;

const int PADDING_SIZE = 3;

std::vector<SzablonObrazu> wczytajSzablonyZnakowObrazy(const std::string& folderGlownySzablonow, const std::vector<char>& alfabet) {
    std::vector<SzablonObrazu> bazaSzablonow;
    std::cout << "Wczytywanie szablonów (obrazów) z folderu: " << folderGlownySzablonow << std::endl;
    std::map<char, int> pokazaneSzablonyDlaZnakuCount;
    for (const char& aktualnyZnak : alfabet) {
        std::string nazwaFolderuZnaku;
        if (aktualnyZnak == '.') nazwaFolderuZnaku = "dot";
        else if (aktualnyZnak == ',') nazwaFolderuZnaku = "comma";
        else if (aktualnyZnak == '?') nazwaFolderuZnaku = "question";
        else if (aktualnyZnak == '!') nazwaFolderuZnaku = "exclamation";
        else nazwaFolderuZnaku = std::string(1, aktualnyZnak);
        std::string sciezkaDoFolderuZnaku = folderGlownySzablonow + "/" + nazwaFolderuZnaku;
        if (fs::exists(sciezkaDoFolderuZnaku) && fs::is_directory(sciezkaDoFolderuZnaku)) {
            for (const auto& wpis : fs::directory_iterator(sciezkaDoFolderuZnaku)) {
                if (wpis.is_regular_file()) {
                    std::string sciezkaObrazkaSzablonu = wpis.path().string();
                    cv::Mat obrazSzablonuOryginal = cv::imread(sciezkaObrazkaSzablonu, cv::IMREAD_GRAYSCALE);
                    if (!obrazSzablonuOryginal.empty()) {
                        cv::Mat szablonBinarnyFormatPorownania;
                        cv::threshold(obrazSzablonuOryginal, szablonBinarnyFormatPorownania, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
                        cv::Mat szablonZPaddingiem = stworzObrazZPaddingiem(szablonBinarnyFormatPorownania, cv::Size(32,32), PADDING_SIZE);
                        SzablonObrazu szablon; szablon.znak = aktualnyZnak; szablon.obraz32x32 = szablonZPaddingiem;
                        bazaSzablonow.push_back(szablon);
                    } else { std::cerr << "    Błąd wczytania obrazka szablonu: " << sciezkaObrazkaSzablonu << std::endl; }
                }
            }
        } else { std::cerr << "  Ostrzeżenie: Folder szablonów '" << sciezkaDoFolderuZnaku << "' nie istnieje." << std::endl; }
    }
    std::cout << "Wczytano " << bazaSzablonow.size() << " szablonów obrazów." << std::endl;
    return bazaSzablonow;
}

cv::Mat stworzObrazZPaddingiem(const cv::Mat& obrazWejsciowyBinarny, const cv::Size& docelowyRozmiarFinalny, int padding) {

    cv::Mat obrazWynikowyZPaddingiem;
    int docelowaSzerokoscWewn = docelowyRozmiarFinalny.width - 2 * padding;
    int docelowaWysokoscWewn = docelowyRozmiarFinalny.height - 2 * padding;

    if (docelowaSzerokoscWewn <= 0 || docelowaWysokoscWewn <= 0 || obrazWejsciowyBinarny.empty() || obrazWejsciowyBinarny.cols == 0 || obrazWejsciowyBinarny.rows == 0) {

        cv::resize(obrazWejsciowyBinarny, obrazWynikowyZPaddingiem, docelowyRozmiarFinalny, 0, 0, cv::INTER_AREA);
        if (obrazWynikowyZPaddingiem.type() != CV_8U) {

            obrazWynikowyZPaddingiem.convertTo(obrazWynikowyZPaddingiem, CV_8U);

        }

        return obrazWynikowyZPaddingiem;

    }

    cv::Mat obrazPrzeskalowany;
    double r = std::min((double)docelowaSzerokoscWewn / obrazWejsciowyBinarny.cols, (double)docelowaWysokoscWewn / obrazWejsciowyBinarny.rows);
    int nowaSzerokosc = std::max(1, (int)std::round(obrazWejsciowyBinarny.cols * r));
    int nowaWysokosc = std::max(1, (int)std::round(obrazWejsciowyBinarny.rows * r));
    cv::Size nowyRozmiarZnaku(nowaSzerokosc, nowaWysokosc);

    cv::resize(obrazWejsciowyBinarny, obrazPrzeskalowany, nowyRozmiarZnaku, 0, 0, cv::INTER_AREA);
    obrazWynikowyZPaddingiem = cv::Mat::zeros(docelowyRozmiarFinalny, obrazWejsciowyBinarny.type());
    int xOffset = (docelowyRozmiarFinalny.width - obrazPrzeskalowany.cols) / 2;
    int yOffset = (docelowyRozmiarFinalny.height - obrazPrzeskalowany.rows) / 2;
    cv::Rect roiDocelowy(xOffset, yOffset, obrazPrzeskalowany.cols, obrazPrzeskalowany.rows);
    if (roiDocelowy.x < 0) roiDocelowy.x = 0; if (roiDocelowy.y < 0) roiDocelowy.y = 0;
    if (roiDocelowy.x + roiDocelowy.width > obrazWynikowyZPaddingiem.cols) roiDocelowy.width = obrazWynikowyZPaddingiem.cols - roiDocelowy.x;
    if (roiDocelowy.y + roiDocelowy.height > obrazWynikowyZPaddingiem.rows) roiDocelowy.height = obrazWynikowyZPaddingiem.rows - roiDocelowy.y;

    if (roiDocelowy.width > 0 && roiDocelowy.height > 0 && obrazPrzeskalowany.cols > 0 && obrazPrzeskalowany.rows > 0 &&
        roiDocelowy.width <= obrazPrzeskalowany.cols && roiDocelowy.height <= obrazPrzeskalowany.rows) {
        cv::Mat srcForCopy = obrazPrzeskalowany(cv::Rect(0,0, roiDocelowy.width, roiDocelowy.height));
        srcForCopy.copyTo(obrazWynikowyZPaddingiem(roiDocelowy));
    } else {
        cv::resize(obrazWejsciowyBinarny, obrazWynikowyZPaddingiem, docelowyRozmiarFinalny, 0, 0, cv::INTER_AREA);
    }
    if (obrazWynikowyZPaddingiem.type() != CV_8U) obrazWynikowyZPaddingiem.convertTo(obrazWynikowyZPaddingiem, CV_8U);

    return obrazWynikowyZPaddingiem;

}

cv::Mat wczytajIPrzetworzWstepnie(const std::string& sciezkaObrazu, bool& sukces) {

    sukces = true;
    cv::Mat obraz = cv::imread(sciezkaObrazu, cv::IMREAD_COLOR);
    if (obraz.empty()) {

        std::cerr << "Błąd: Nie można wczytać obrazu: " << sciezkaObrazu << std::endl;
        sukces = false;
        return cv::Mat();

    }
    cv::Mat obrazSzary;
    cv::cvtColor(obraz, obrazSzary, cv::COLOR_BGR2GRAY);
    cv::Mat obrazRozmyty;
    cv::GaussianBlur(obrazSzary, obrazRozmyty, cv::Size(3, 3), 0);
    cv::Mat obrazDoBinaryzacji = obrazRozmyty.clone();
    cv::Mat obrazBinarny;
    cv::threshold(obrazDoBinaryzacji, obrazBinarny, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);

    return obrazBinarny;
}

std::vector<cv::Mat> segmentujNormalizujZnaki(const cv::Mat& obrazBinarnyWejsciowy, const cv::Mat& obrazSzaryOryginal, const cv::Mat& obrazKolorowyOryginal, std::vector<cv::Rect>& pozycjeZnakowOriginal) {

    std::vector<cv::Mat> wysegmentowaneZnakiDlaPorownania;
    pozycjeZnakowOriginal.clear();

    cv::Mat obrazBinarnyDoKonturow = obrazBinarnyWejsciowy.clone();

    std::vector<std::vector<cv::Point>> kontury;
    std::vector<cv::Vec4i> hierarchia;
    cv::findContours(obrazBinarnyDoKonturow, kontury, hierarchia, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<cv::Rect> ramkiPierwotne;
    for (const auto& kontur : kontury) {
        ramkiPierwotne.push_back(cv::boundingRect(kontur));
    }
    std::vector<bool> przetworzone(ramkiPierwotne.size(), false);
    std::vector<cv::Rect> ramkiFinalne;

    for (size_t i = 0; i < ramkiPierwotne.size(); ++i) {
        if (przetworzone[i]) continue;
        cv::Rect trzonPotencjalny = ramkiPierwotne[i];

        bool czyMozeBycTrzonem = (trzonPotencjalny.height > 20 && trzonPotencjalny.height < 250 && trzonPotencjalny.width > 3   && trzonPotencjalny.width < 100  && trzonPotencjalny.width < static_cast<int>(trzonPotencjalny.height * 0.8) && trzonPotencjalny.area() > 50);

        bool znalezionoKropkeDlaTrzonu = false;
        if (czyMozeBycTrzonem) {
            for (size_t j = 0; j < ramkiPierwotne.size(); ++j) {
                if (i == j || przetworzone[j]) continue;
                cv::Rect kropkaPotencjalna = ramkiPierwotne[j];

                bool czyMozeBycKropka = (kropkaPotencjalna.area() >= 100 && kropkaPotencjalna.area() < 1500 && kropkaPotencjalna.height >= 15 && kropkaPotencjalna.height < 60 && kropkaPotencjalna.width >= 15 && kropkaPotencjalna.width < 60 && std::abs(kropkaPotencjalna.width - kropkaPotencjalna.height) < 20);


                if (czyMozeBycKropka) {
                    int srodekXTrzonu = trzonPotencjalny.x + trzonPotencjalny.width / 2;
                    int srodekXKropki = kropkaPotencjalna.x + kropkaPotencjalna.width / 2;
                    int odlegloscPionowaDolKropkiDoGoryTrzonu = trzonPotencjalny.y - (kropkaPotencjalna.y + kropkaPotencjalna.height);
                    int tolerancjaX = std::min(trzonPotencjalny.width, 35);

                    bool warunkiGeoSpelnione = (kropkaPotencjalna.y < trzonPotencjalny.y && std::abs(srodekXTrzonu - srodekXKropki) < tolerancjaX * 1.5 && odlegloscPionowaDolKropkiDoGoryTrzonu > -10 && odlegloscPionowaDolKropkiDoGoryTrzonu < static_cast<int>(trzonPotencjalny.height * 0.8));

                    if (warunkiGeoSpelnione) {
                        cv::Rect polaczonaRamka = trzonPotencjalny | kropkaPotencjalna;
                        ramkiFinalne.push_back(polaczonaRamka);
                        przetworzone[i] = true; przetworzone[j] = true;
                        znalezionoKropkeDlaTrzonu = true;
                        break;
                    }
                }
            }
        }
        if (!znalezionoKropkeDlaTrzonu && !przetworzone[i]) {
            ramkiFinalne.push_back(trzonPotencjalny);
            przetworzone[i] = true;
        }
    }
    for(size_t i=0; i < ramkiPierwotne.size(); ++i) {
        if(!przetworzone[i]) {
            ramkiFinalne.push_back(ramkiPierwotne[i]);
        }
    }

    std::vector<std::pair<cv::Rect, cv::Mat>> kandydaciNaZnaki;
    int przepuszczonePoFiltrowaniuCount = 0;
    for (const auto& ramkaOgraniczajaca : ramkiFinalne) {
        if (ramkaOgraniczajaca.area() > 2 && ramkaOgraniczajaca.height > 2 && ramkaOgraniczajaca.width > 1 &&
            ramkaOgraniczajaca.width < obrazBinarnyWejsciowy.cols &&
            ramkaOgraniczajaca.height < obrazBinarnyWejsciowy.rows) {
            przepuszczonePoFiltrowaniuCount++;
            if (ramkaOgraniczajaca.x >= 0 && ramkaOgraniczajaca.y >= 0 &&
                ramkaOgraniczajaca.x + ramkaOgraniczajaca.width <= obrazSzaryOryginal.cols &&
                ramkaOgraniczajaca.y + ramkaOgraniczajaca.height <= obrazSzaryOryginal.rows) {
                cv::Mat roiZnakSzary = obrazSzaryOryginal(ramkaOgraniczajaca);
                cv::Mat roiZnakBinarnyFormatDft;
                cv::threshold(roiZnakSzary, roiZnakBinarnyFormatDft, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
                cv::Mat znakZPaddingiem = stworzObrazZPaddingiem(roiZnakBinarnyFormatDft, cv::Size(32,32), PADDING_SIZE);
                kandydaciNaZnaki.push_back({ramkaOgraniczajaca, znakZPaddingiem});
            }
        }
    }

    std::sort(kandydaciNaZnaki.begin(), kandydaciNaZnaki.end(),
                        [](const std::pair<cv::Rect, cv::Mat>& paraA, const std::pair<cv::Rect, cv::Mat>& paraB) {

                        const cv::Rect& a = paraA.first;
                        const cv::Rect& b = paraB.first;

                        int srodek_y_a = a.y + a.height / 2;
                        int srodek_y_b = b.y + b.height / 2;

                        int mniejszaWysokosc = std::min(a.height, b.height);
                        int akceptowalnaRoznicaY = std::max(170, mniejszaWysokosc / 2);

                        if (std::abs(srodek_y_a - srodek_y_b) > akceptowalnaRoznicaY) {
                            return srodek_y_a < srodek_y_b;
                        } else {
                            return a.x < b.x;
                        }
                    });
    for(const auto& para : kandydaciNaZnaki) {
        wysegmentowaneZnakiDlaPorownania.push_back(para.second);
        pozycjeZnakowOriginal.push_back(para.first);
    }
    return wysegmentowaneZnakiDlaPorownania;
}

char rozpoznajZnakPrzezMatchTemplate(const cv::Mat& obrazSegmentowany32x32, const std::vector<SzablonObrazu>& bazaSzablonowObrazow, int segmentIndexForDebug = -1) {
    if (obrazSegmentowany32x32.empty() || obrazSegmentowany32x32.size() != cv::Size(32,32) || obrazSegmentowany32x32.type() != CV_8U) return '#';
    if (bazaSzablonowObrazow.empty()) return '#';
    char najlepszyZnak = '#';
    double maxWspolczynnikKorelacji = -2.0;
    for (const auto& szablon : bazaSzablonowObrazow) {
        if (szablon.obraz32x32.empty() || szablon.obraz32x32.size() != obrazSegmentowany32x32.size() || szablon.obraz32x32.type() != obrazSegmentowany32x32.type()) continue;
        cv::Mat wynikPorownania;
        cv::matchTemplate(obrazSegmentowany32x32, szablon.obraz32x32, wynikPorownania, cv::TM_CCOEFF_NORMED);
        double wspolczynnikKorelacji = wynikPorownania.at<float>(0,0);
        if (wspolczynnikKorelacji > maxWspolczynnikKorelacji) {
            maxWspolczynnikKorelacji = wspolczynnikKorelacji; najlepszyZnak = szablon.znak;
        }
    }

    return najlepszyZnak;
}

std::string rekonstruujTekst(const std::vector<char>& rozpoznaneZnaki, const std::vector<cv::Rect>& pozycjeZnakow) {
    if (rozpoznaneZnaki.empty()) return "";
    if (pozycjeZnakow.empty() || rozpoznaneZnaki.size() != pozycjeZnakow.size()) {
        std::string s = "";
        for (char c : rozpoznaneZnaki) if (c != '#') s += c;
        return s;
    }

    std::string zrekonstruowanyTekst = "";

    if (rozpoznaneZnaki[0] != '#') {
        zrekonstruowanyTekst += rozpoznaneZnaki[0];
    }

    double sredniaSzerokoscZnaku = 0;
    int iloscPrawidlowychZnakowDlaSzerokosci = 0;
    for(size_t i=0; i < rozpoznaneZnaki.size(); ++i) {
        if(rozpoznaneZnaki[i] != '#') {
             sredniaSzerokoscZnaku += pozycjeZnakow[i].width;
             iloscPrawidlowychZnakowDlaSzerokosci++;
        }
    }
    if (iloscPrawidlowychZnakowDlaSzerokosci > 0) {
        sredniaSzerokoscZnaku /= iloscPrawidlowychZnakowDlaSzerokosci;
    } else {
        sredniaSzerokoscZnaku = 10;
    }

    if (sredniaSzerokoscZnaku < 5) {
        sredniaSzerokoscZnaku = 10;
    }


    for (size_t i = 1; i < rozpoznaneZnaki.size(); ++i) {
        const cv::Rect& popZnakRect = pozycjeZnakow[i-1];
        const char& popZnakChar = rozpoznaneZnaki[i-1];
        const cv::Rect& aktZnakRect = pozycjeZnakow[i];
        const char& aktZnakChar = rozpoznaneZnaki[i];

        if (aktZnakRect.y > popZnakRect.y + popZnakRect.height * 1.5) {
            if (!zrekonstruowanyTekst.empty() && zrekonstruowanyTekst.back() != '\n') {
                zrekonstruowanyTekst += '\n';
            }
        }
        else if (popZnakChar != '#' &&
                 aktZnakChar != '.' && aktZnakChar != ',' && aktZnakChar != '?' && aktZnakChar != '!' &&
                 aktZnakRect.x > popZnakRect.x + popZnakRect.width + sredniaSzerokoscZnaku * 0.30) {
            if (!zrekonstruowanyTekst.empty() && zrekonstruowanyTekst.back() != ' ' && zrekonstruowanyTekst.back() != '\n') {
                zrekonstruowanyTekst += ' ';
            }
        }

        if (aktZnakChar != '#') {
            zrekonstruowanyTekst += aktZnakChar;
        }
    }

    return zrekonstruowanyTekst;

}

int main() {

    std::cout << "Podaj nazwę pliku w folderze img:\t";
    std::string buf;
    std::cin >> buf;
    if (buf.length() != 0) {
        IMAGE_NAME = buf;
    }
    std::cout << "Jeżeli chcesz zobaczyć poprawność OCR podaj tekst, któy jest na obrazku:\t";
    buf.clear();
    std::cin >> buf;
    if (buf.length() != 0) {
        tekstGroundTruth = buf;
    }

    std::string folderSzablonow = "../assets";
    std::vector<char> alfabetDoRozpoznania = {
        'a','b','c','d','e','f','g','h','i','j','k','l','m',
        'n','o','p','q','r','s','t','u','v','w','x','y','z',
        '0','1','2','3','4','5','6','7','8','9',
        '.', ',', '?', '!'
    };
    std::vector<SzablonObrazu> bazaSzablonowObrazow = wczytajSzablonyZnakowObrazy(folderSzablonow, alfabetDoRozpoznania);
    if (bazaSzablonowObrazow.empty()) { std::cerr << "Krytyczny błąd: Nie wczytano żadnych szablonów obrazów." << std::endl; return -1; }

    std::string sciezkaDoObrazuTestowego = IMAGE_FOLDER + IMAGE_NAME;

    std::cout << "\n--- Przetwarzanie obrazu testowego: " << sciezkaDoObrazuTestowego << " ---" << std::endl;
    bool sukcesWczytania;
    cv::Mat obrazWejsciowyOryginalKolor = cv::imread(sciezkaDoObrazuTestowego, cv::IMREAD_COLOR);
    cv::Mat obrazWejsciowyOryginalSzary;
    if(!obrazWejsciowyOryginalKolor.empty()) {
        cv::cvtColor(obrazWejsciowyOryginalKolor, obrazWejsciowyOryginalSzary, cv::COLOR_BGR2GRAY);
    }
    else {
        std::cerr << "Nie udało się wczytać obrazu testowego: " << sciezkaDoObrazuTestowego << std::endl;
        return -1;
    }

    cv::Mat obrazBinarnyPrzetworzony = wczytajIPrzetworzWstepnie(sciezkaDoObrazuTestowego, sukcesWczytania);
    if (!sukcesWczytania || obrazBinarnyPrzetworzony.empty()) {
        std::cerr << "Nie udało się wczytać lub przetworzyć obrazu testowego." << std::endl;
        return -1;
    }

    std::vector<cv::Rect> pozycjeRozpoznanychZnakow;
    std::vector<cv::Mat> znormalizowaneZnakiDoPorownania = segmentujNormalizujZnaki(obrazBinarnyPrzetworzony, obrazWejsciowyOryginalSzary, obrazWejsciowyOryginalKolor, pozycjeRozpoznanychZnakow);
    std::vector<char> rozpoznaneZnakiLista;
    std::map<char, int> licznikLiter;
    std::cout << "Liczba wysegmentowanych znaków do rozpoznania: " << znormalizowaneZnakiDoPorownania.size() << std::endl;

    for (size_t i = 0; i < znormalizowaneZnakiDoPorownania.size(); ++i) {
        const auto& znormalizowanyZnak = znormalizowaneZnakiDoPorownania[i];
        char odczytanyZnak = rozpoznajZnakPrzezMatchTemplate(znormalizowanyZnak, bazaSzablonowObrazow, static_cast<int>(i));
        rozpoznaneZnakiLista.push_back(odczytanyZnak);
        if (odczytanyZnak != '#') if ((odczytanyZnak >= 'a' && odczytanyZnak <= 'z')) {
            licznikLiter[odczytanyZnak]++;
        }
    }

    std::string finalnyRozpoznanyTekst = rekonstruujTekst(rozpoznaneZnakiLista, pozycjeRozpoznanychZnakow);

    std::cout << "------------------------------------" << std::endl;
    std::cout << "Rozpoznany tekst (zrekonstruowany): \n" << finalnyRozpoznanyTekst << std::endl;
    std::cout << "------------------------------------" << std::endl;
    std::cout << "Licznik wystąpień liter (a-z):" << std::endl;

    for (char c = 'a'; c <= 'z'; ++c) {
        if (licznikLiter.count(c)) {
            std::cout << "'" << c << "': " << licznikLiter[c] << std::endl;
        }
    }
    std::cout << "------------------------------------" << std::endl;

    if (!tekstGroundTruth.empty()) {
        int poprawneZnakiCounter = 0; std::string gtClean = tekstGroundTruth; std::string recClean = finalnyRozpoznanyTekst;
        gtClean.erase(std::remove_if(gtClean.begin(), gtClean.end(), [](unsigned char x){return std::isspace(x);}), gtClean.end());
        recClean.erase(std::remove_if(recClean.begin(), recClean.end(), [](unsigned char x){return std::isspace(x);}), recClean.end());
        int minDlugosc = std::min((int)recClean.length(), (int)gtClean.length());
        for (int i = 0; i < minDlugosc; ++i) {
            if (recClean[i] == gtClean[i]) {
                poprawneZnakiCounter++;
            }
        }
        double dokladnosc = (gtClean.empty() ? 0.0 : (static_cast<double>(poprawneZnakiCounter) / gtClean.length()) * 100.0);
        std::cout << "Procent poprawnie rozpoznanych znaków (vs ground truth, ignorując białe znaki): " << dokladnosc << "%" << std::endl;

    } else {

        std::cout << "Brak tekstu ground truth do porównania dokładności." << std::endl;

    }
    std::cout << "------------------------------------" << std::endl;

    cv::Mat obrazWynikowy = obrazWejsciowyOryginalKolor.clone();
    for(size_t i=0; i < pozycjeRozpoznanychZnakow.size(); ++i) {
        cv::rectangle(obrazWynikowy, pozycjeRozpoznanychZnakow[i], cv::Scalar(0,255,0),1);
        if (i < rozpoznaneZnakiLista.size() && rozpoznaneZnakiLista[i] != '#') {
            cv::putText(obrazWynikowy, std::string(1, rozpoznaneZnakiLista[i]), cv::Point(pozycjeRozpoznanychZnakow[i].x, pozycjeRozpoznanychZnakow[i].y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,0,0), 1);
        }
        else if (i < rozpoznaneZnakiLista.size()) {
            cv::putText(obrazWynikowy, "#", cv::Point(pozycjeRozpoznanychZnakow[i].x, pozycjeRozpoznanychZnakow[i].y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,255), 1);
        }
    }
    cv::imshow("Rozpoznane Znaki na Obrazie", obrazWynikowy);
    size_t last_dot = IMAGE_NAME.rfind('.');
    std::string base_name = IMAGE_NAME.substr(0, last_dot);
    cv::imwrite(IMAGE_FOLDER + base_name + "_result.png", obrazWynikowy);
    cv::waitKey(0);

    return 0;
}