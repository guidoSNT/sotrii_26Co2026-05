# Paso 02 - Cyclic scheduling

## Sistema 1

| Tarea | C   | T = D |
| :---- | :-- | :---: |
| T1    | 1   |   4   |
| T2    | 2   |   5   |
| T3    | 5   |  20   |

### Factor de uso

U = 1/4 + 2/5 + 5/20 = 0,9

### Hiperperiodo

H = T_m = mcm(4,5,20) = 20

### Periodo secundario

T_s = mcd(4,5,20) = 1 -> eligo 5

### Test de garantia

1. T_s = 5 >= max(C) = 5 => CUMPLE
2. U = 0,9 <= 1 => CUMPLE
3. T_m = mcm(4,5,20) = 20 => CUMPLE
4. T_i = k T_s / k e N. Como el periodo secundario es 1, no existe ningun numero entero k que multiplicado por el T_s de los periodos 4, 5 y 20.
5. 2 T_s - mcd(T_s,T_i) <= T_i:
   T1: 2 * 5 - mcd(5,4) = 9 <= 4 -> NO CUMPLE
   T2: 2 * 5 - mcd(5,5) = 5 <= 5 -> CUMPLE
   T3: 2 * 5 - mcd(5,20) = 5 <= 20 -> CUMPLE

No cumple por condicion 1.

## Sistema 2

| Tarea | C   | T = D |
| :---- | :-- | :---: |
| T1    | 1   |   6   |
| T2    | 2   |  10   |
| T3    | 2   |  18   |

### Factor de uso

U = 1/6 + 2/10 + 2/18 = 0,478

### Hiperperiodo

H = T_m = mcm(6, 10, 18) = 90

### Periodo secundario

T_s = mcd(6, 10, 18) = 2

### Test de garantia

1. T_s = 2 >= max(C) = 2 => CUMPLE
2. U = 0,478 <= 1 => CUMPLE
3. T_m = mcm(6, 10, 18) = 90 => CUMPLE
4. T_i = k T_s / k e N => (T_s * 3 = T_1 = 6 , T_s * 5 = T_2 = 10, T_s * 9 = T_3 = 18)
5. 2 T_s - mcd(T_s,T_i) <= T_i:
   T1: 2 * 2 - mcd(2, 6) = 2 <= 6 -> Cumple
   T2: 2 * 2 - mcd(2, 10) = 2 <= 10 -> Cumple
   T3: 2 * 2 - mcd(2, 18) = 2 <= 18 -> Cumple

Este sistema si cumple el test de garantia.

## Sistema 3

| Tarea | C   | T = D |
| :---- | :-- | :---: |
| T1    | 1   |   8   |
| T2    | 3   |  15   |
| T3    | 4   |  20   |
| T4    | 6   |  22   |

### Factor de uso

U = 1/8 + 3/15 + 4/20 + 6/22 = 0,797

### Hiperperiodo

H = T_m = mcm(8, 15, 20, 22) = 1320

### Periodo secundario

T_s = mcd(8, 15, 20, 22) = 1 -> Eligo 6

### Test de garantia

1. T_s = 6 >= max(C) = 6 => CUMPLE
2. U = 0,797 <= 1 => CUMPLE
3. T_m = mcm(8, 15, 20, 22) = 1320 => CUMPLE
4. T_i = k T_s / k e N => No cumple ya que 8 no es multiplo de 6
5. 2 T_s - mcd(T_s,T_i) <= T_i:
   T1: 2 * 1 - mcd(1, 8) = 1 <= 8 -> Cumple
   T2: 2 * 1 - mcd(1, 15) = 1 <= 15 -> Cumple
   T3: 2 * 1 - mcd(1, 20) = 1 <= 20 -> Cumple
   T4: 2 * 1 - mcd(1, 22) = 1 <= 22 -> Cumple

Este sistema no cumple por la condicion 1.

## Sistema 4

| Tarea |  C  | T = D |
| :---- | :-: | :---: |
| T1    | 0,5 |   4   |
| T2    |  1  |   5   |
| T3    |  2  |  10   |
| T4    |  9  |  24   |

### Factor de uso

U = 0,5/4 + 1/5 + 2/10 + 9/24 = 0,9

### Hiperperiodo

H = T_m = mcm(4, 5, 10, 24) = 120

### Periodo secundario

T_s = mcd(4, 5, 10, 24) = 1 -> Eligo 9 para satifacer condicion 1

### Test de garantia

1. T_s = 9 >= max(C) = 9 => CUMPLE
2. U = 0,9 <= 1 => CUMPLE
3. T_m = mcm(4, 5, 10, 24) = 120 => CUMPLE
4. T_i = k T_s / k e N => No cumple porque ningun de los periodos es multiplo de T_s
5. 2 T_s - mcd(T_s,T_i) <= T_i:
   T1: 2 * 9 - mcd(9, 4) = 17 <= 3 -> No cumple
   T2: 2 * 9 - mcd(9, 5) = 17 <= 5 -> No cumple
   T3: 2 * 9 - mcd(9, 10) = 17 <= 10 -> No cumple
   T4: 2 * 9 - mcd(9, 24) = 15 <= 24 -> Cumple

Este sistema no cumple por la condicion 4.

# Modificaciones para usar con FreeRTOS

Para poder usar esto con FreeRTOS, se debe deshabilitar `USE_PREEMPTION` que es un define del `FreeRTOS_config.h`, aunque desde STM32CubeIDE se pude hacer desde el IOC.
