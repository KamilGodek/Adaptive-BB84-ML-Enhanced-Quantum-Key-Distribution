import pandas as pd
import matplotlib.pyplot as plt
from sklearn.model_selection import train_test_split
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import accuracy_score, confusion_matrix, classification_report
import numpy as np

# 1.Wczytanie Danych 

try:
    data = pd.read_csv('E:/AI-Aided_BB84/data/training_data.csv')
    print("Dane zostały wczytane pomyślnie.")
    print("\n Pierwsze 5 wierszy danych:")
    print(data.head())
    print(f"\nCałkowita liczba probek: {len(data)}")

except FileNotFoundError:
    print("Błąd: Plik 'training_data.csv' nie został znaleziony. Upewnij się, że symulacja C++ została uruchomiona i plik istnieje.")
    exit()

# 2.Analiza wstępna (wizualiuzacja)

plt.figure(figsize=(10, 6))

# Histogram dla "czystego" kanału (Attack_Detected == 0)
plt.hist(data[data['Attack_Detected'] == 0]['QBER_Value'] * 100, 
         bins=20, alpha=0.6, label='Brak Ataku (Szum naturalny)', color='blue')


#Histogram dla "atakowanego kanalu" (Attack_Detected ==1)
plt.hist(data[data['Attack_Detected'] == 1]['QBER_Value']* 100,
         bins=20, alpha = 0.6, label='Atak Eve (Podwyższony szum)', color='red')

plt.title('Rozklad QBER (Współczynnika Błędu Kwantowego)')
plt.xlabel('Wartosc QBER [%]')
plt.ylabel('Częstotliwość')
plt.legend()
plt.grid(axis='y', alpha = 0.5)
plt.show()

print("\nWygenerowano wykres rozkładu QBER. Zamknij wykres, aby kontynuować trening.")


# 3.Przygotowanie danych do treningu

#   X:Zmienne niezalezne (cechy) - tylko QBER, wymaga formatu (N,1), dlatego uzywamy podwójnych nawiasów

X = data[['QBER_Value']]
y = data['Attack_Detected']

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size= 0.2, random_state=42)

print(f"\nRozmiar zbioru treningowego (80%): {len(X_train)} próbek.")
print(f"Rozmiar zbioru testowego (20%): {len(X_test)} próbek.")

# 4. Trening modelu AI z regresją (regresja logistyczna)

#Inicjalizacja modelu i trening modelu 

model = LogisticRegression()
model.fit(X_train, y_train)

print("\n Model AI (Regresja Logistyczna) została pomyslnie wytrenowana")

# 5. Ocena Modelu

y_pred = model.predict(X_test)

accuracy = accuracy_score(y_test, y_pred) * 100
print(f"\nDokladnosc modelu na zbiorze testowym to: {accuracy:.2}%")


#Macierz konfuzji dla lepszej oceny

conf_mat = confusion_matrix(y_test, y_pred)
print("\n Macierz Konfuzji (Confusion Matrix):")
print(conf_mat)
#Szczegolowy raport klasyfikacji

print("\nRaport klasyfikacji (Precyzja, Czulosć):")
print(classification_report(y_test, y_pred))

