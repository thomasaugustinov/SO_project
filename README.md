# Program de Statistici pentru Directoare, Fișiere BMP și Fișiere obișnuite

## Prezentare Generală

Acest program în limbaj C este destinat parcurgerii unui director și generării unui set de statistici pentru fișierele conținute, cu manipulare specială pentru fișierele în format BMP. Programul este executat cu un singur argument care reprezintă calea către director:

./program <director_intrare>


## Cerințe

- Programul necesită exact un argument pentru a rula. Dacă acesta nu este furnizat, se afișează un mesaj de eroare: "Usage: <input_directory>".
- Argumentul trebuie să fie calea către un director existent pe sistemul de fișiere.

## Funcționalități

- Programul parcurge recursiv directorul specificat ca argument.
- Generează un fișier `statistica.txt` în directorul curent, care include detalii despre fiecare element din directorul de intrare:
  - Pentru fișierele BMP: nume, înălțime, lățime, dimensiune, ID-ul utilizatorului, timpul ultimei modificări, numărul de legături, drepturi de acces pentru utilizator, grup și alții.
  - Pentru fișierele obișnuite non-BMP: aceleași informații, excluzând înălțimea și lățimea.
  - Pentru legăturile simbolice: nume legătură, dimensiune legătură, dimensiune fișier target, drepturi de acces pentru utilizator, grup și alții.
  - Pentru directoare: nume director, identificatorul utilizatorului, drepturi de acces pentru utilizator, grup și alții.
  - Pentru alte tipuri de fișiere, nu se scrie nimic în fișierul `statistica.txt`.

## Detalii Implementare

- `FileExtension`: Identifică extensia unui fișier din calea sa.
- `WriteFormattedToFile`: Scrie text formatat într-un fișier, tratând cazurile de eroare.
- `WritePermissionToFile`: Scrie permisiunile unui fișier într-un mod lizibil.
- `SafeRead`: Asigură citirea sigură dintr-un fișier, tratând cazurile de eroare.

## Gestionarea Erorilor

- Programul gestionează erorile întâmpinate în timpul execuției, afișând mesaje relevante și terminând execuția în mod corespunzător pentru a evita corupția de date sau scurgerile de resurse.

## Închiderea Resurselor

- Închiderea resurselor este realizată corespunzător pentru a preveni scurgerile de resurse, indiferent de modul în care programul se termină.




