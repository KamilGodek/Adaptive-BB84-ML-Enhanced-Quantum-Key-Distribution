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

# Histogram dla "atakowanego" kanału (Attack_Detected == 1)
plt.hist(data[data['Attack_Detected'] == 0]['QBER_Value'] * 100, 
         bins=20, alpha=0.6, label='Atak Eve (Szum naturalny)', color='red')


#Histogram dla "atakowanego kanalu" (Attack_Detected ==1)
plt.hist(data[data['Attack_Detected'] == 1]['QBER_Value']* 100,
         bins=20, alpha = 0.6, label='Atak Eve ( podwyższony szum)', color='red')

plt.title('Rozklad QBER (Współczynnika Błędu Kwantowego)')
plt.xlabel('Wartosc QBER [%]')
plt.ylabel('Częstotliwość')
plt.legend()
plt.grid(axis='y', alpha = 0.5)
plt.show()

print("\nWygenerowano wykres rozkładu QBER. Zamknij wykres, aby kontynuować trening.")
