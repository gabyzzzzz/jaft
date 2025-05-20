# Pentru a compila proiectul

    make


# Comenzi esențiale git

## Lucru cu branch-uri 
(creare, ștergere, selecție, redenumire, afișare)

### Creare branch 

varianta mai simplă (fără să schimbe branch-ul pe care ești)
    
    git branch nume-branch

se mai poate și în alt fel, cum ar fi să creezi și să îl și schimbi cu git checkout -b nume-branch, dar pentru simplitate vom folosi git branch nume-branch

### Cum iei branch-uri de pe remote (in afara de main) pe local

intai vezi ce branch-uri remote ai cu comanda
    
    git branch -r 

apoi le iei de pe remote pe local cu 

    git fetch

iar apoi poti sa il selectezi cu numele de pe 'git branch -r' folosind comenzile de mai jos


### Selecția/schimbarea (switch) branch-ului curent

    git switch nume-branch 

sau varianta mai veche 

    git checkout nume-branch

### Ștergere

cum ștergi un branch (nu trebuie să fi pe branch-ul pe care vrei să-l ștergi, chestia asta se face în doi pași)
    
schimbi branch-ul pe care ești (dacă ești în branch-ul pe care vrei să-l ștergi de pe local, adică de pe mașina ta)

    git switch main 

ștergi branch-ul (doar local, de pe remote, adică pe github se face din pull request sau cu o comandă în terminal, dar e maio simplu din pull request pe github, în browser)

    git branch -d nume-branch

### Redenumire

    git branch -m nume-branch nume-nou-de-branch

### Afișare branch

    git branch


## Commit-uri, înainte de push, fac valabile schimbările la următorul push pe remote (origin)

### trebuie să vezi statusul întâi

    git status

### după adaugi ce vrei să intre în commit (asigurându-te la finalul comenzii cu git status că s-au adăugat)

    git add nume-fișier

### commit-ul în sine

    git commit -m "mesaj commit"


## Push, Pull, no legs 
(cum pui de pe mașina locală pe remote și cum iei de pe aia remote și pui pe aia locală)

### Cum updatezi pe mașina locală (PULL)

    git pull

### Cum pui pe cea remote, pe github 
(PUSH) un branch, nu dai push lui main decât dacă sunt lucruri care sigur merg

    git push -u origin nume-branch

(dai pull request copying link-ul pe care ti-l da dupa comanda asta)


apoi schimbi pe main cu 

    git switch main

și ștergi nume-branch (daca ai dat merge dupa ce ai dat pull request pe github, altfel nu trebuie sa il stergi si poti continua sa lucrezi pe el)

    git branch -d nume-branch

