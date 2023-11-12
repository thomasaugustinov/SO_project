# Program de Statistici pentru Fișiere BMP

## Prezentare Generală
Acest program în limbaj C prelucrează un fișier de intrare care reprezintă o imagine în format BMP pentru a genera un set de statistici. Programul este executat cu un singur argument după cum urmează:

./program <fisier_intrare>


## Cerințe
- Programul necesită exact un argument pentru a rula. Dacă acesta nu este furnizat, se afișează un mesaj de eroare: "Utilizare ./program <fisier_intrare>".
- Fișierul de intrare trebuie să fie o imagine BMP. Programul verifică extensia fișierului și iese cu o eroare dacă extensia nu este `.bmp`.

## Funcționalități
- Programul citește antetul fișierului BMP pentru a extrage lățimea și înălțimea imaginii.
- Generează un fișier denumit `statistica.txt`, care include următoarele detalii:
  - Numele fișierului
  - Înălțimea imaginii
  - Lățimea imaginii
  - Dimensiunea fișierului în octeți
  - ID-ul utilizatorului proprietar
  - Timpul ultimei modificări
  - Numărul de legături către fișier
  - Drepturi de acces pentru utilizator, grup și alții

## Detalii Implementare
- `FileExtension`: Validează calea fișierului și asigură că argumentul este un fișier cu o extensie.
- `WriteFormattedToFile`: Scrie șiruri de caractere formatate într-un descriptor de fișier, gestionând erorile și închizând fișierul dacă scrierea eșuează.
- `WritePermissionToFile`: Scrie informațiile despre permisiunile fișierului într-un format ușor de citit.
- `SafeRead`: Citește în siguranță date din fișier, asigurându-se de gestionarea corespunzătoare a erorilor și închiderea fișierului în caz de eșec al citirii.
- `main`: Punctul de intrare al programului care orchestrează citirea din fișierul BMP, extragerea statisticilor și scrierea în fișierul `statistica.txt`.

## Gestionarea Erorilor
Programul include o gestionare robustă a erorilor pentru operațiunile cu fișiere. Dacă orice apel de sistem eșuează, programul afișează un mesaj de eroare relevant, eliberează resursele și iese pentru a evita comportamentul nedefinit sau scurgerile de resurse.

## Închiderea Resurselor
La finalizare sau în caz de eroare, programul asigură că toate descriptoarele de fișiere sunt închise corespunzător pentru a preveni scurgerile de resurse.
