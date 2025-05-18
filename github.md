# Comenzi esențiale git

## Lucru cu branch-uri (creare, ștergere, selecție, redenumire, afișare)

### Creare branch 

    varianta mai simplă (fără să schimbe branch-ul pe care ești)
    
    'git branch nume-branch'

    se mai poate și în alt fel, cum ar fi să creezi și să îl și schimbi cu 'git checkout -b nume-branch', dar pentru simplitate vom folosi 'git branch nume-branch'

### Selecția/schimbarea (switch) branch-ului curent

    'git switch nume-branch' sau varianta mai veche 'git checkout nume-branch'

### Ștergere

    cum ștergi un branch (nu trebuie să fi pe branch-ul pe care vrei să-l ștergi, chestia asta se face în doi pași)
    
    schimbi branch-ul pe care ești (dacă ești în branch-ul pe care vrei să-l ștergi de pe local, adică de pe mașina ta)

    'git switch main' 

    ștergi branch-ul

    'git branch -d nume-branch'

### Redenumire

    'git branch -m nume-branch nume-nou-de-branch'

### Afișare branch

    'git branch'
