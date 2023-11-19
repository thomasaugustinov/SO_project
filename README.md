# Program de Statistici pentru Directoare, Fișiere BMP și Fișiere obișnuite

## Prezentare Generală

Acest program în limbaj C este destinat parcurgerii unui director și generării unui set de statistici pentru fișierele conținute, cu manipulare specială pentru fișierele în format BMP. Programul este executat cu două argumente care reprezintă calea către directorul de intrare și calea către directorul de ieșire:

```bash
./program <director_intrare> <director_iesire>
```

## Cerințe

- Programul necesită exact două argumente pentru a rula. Dacă acestea nu sunt furnizate, se afișează un mesaj de eroare: "Usage: ./program <director_intrare> <director_iesire>".
- Argumentele trebuie să fie căi către directoare existente pe sistemul de fișiere.

## Funcționalități

- Programul parcurge recursiv directorul de intrare specificat ca prim argument.
- Generează un fișier <nume_intrare>_statistica.txt în directorul de ieșire pentru fiecare intrare din directorul de intrare, care include urmatoarele detalii:
  - Pentru fișierele BMP: nume, înălțime, lățime, dimensiune, ID-ul utilizatorului, timpul ultimei modificări, numărul de legături, drepturi de acces pentru utilizator, grup și alții.
  - Pentru fișierele obișnuite non-BMP: aceleași informații, excluzând înălțimea și lățimea.
  - Pentru legăturile simbolice: nume legătură, dimensiune legătură, dimensiune fișier target, drepturi de acces pentru utilizator, grup și alții.
  - Pentru directoare: nume director, identificatorul utilizatorului, drepturi de acces pentru utilizator, grup și alții.
  - Pentru alte tipuri de fișiere, nu se scrie nimic în fișierul `statistica.txt`.
- Pentru fișierele BMP, se creează un proces adițional care convertește imaginea în tonuri de gri folosind formula dată.

## Detalii Implementare

- `FileExtension`: Identifică extensia unui fișier din calea sa.
- `WriteFormattedToFile`: Scrie text formatat într-un fișier, tratând cazurile de eroare.
- `WritePermissionToFile`: Scrie permisiunile unui fișier într-un mod lizibil.
- `SafeRead`: Asigură citirea sigură dintr-un fișier, tratând cazurile de eroare.
- `countLines`: Contorizează liniile dintr-un fișier.

## Gestionarea Erorilor

- Programul gestionează erorile întâmpinate în timpul execuției, afișând mesaje relevante și terminând execuția în mod corespunzător pentru a evita corupția de date sau scurgerile de resurse.

## Închiderea Resurselor

- Închiderea resurselor este realizată corespunzător pentru a preveni scurgerile de resurse, indiferent de modul în care programul se termină.

## Note Suplimentare

- La încheierea fiecărui proces fiu, acesta va trimite părintelui numărul de linii scrise în fișierul statistica.txt.
- La terminarea fiecărui proces, părintele va afișa un mesaj corespunzător statusului de încheiere a procesului fiu.




