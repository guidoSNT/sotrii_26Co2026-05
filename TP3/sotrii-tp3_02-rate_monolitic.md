# Paso 02 - Rate Monolitic

Las prioridades se enumeran de mayor a menor siendo el numero mas grande la maxima prioridad.

## Sistema 1

| Tarea | C   | T = D | Prioridades |
| :---- | :-- | :---: | :---------: |
| T1    | 1   |   4   |      3      |
| T2    | 2   |   5   |      2      |
| T3    | 5   |  20   |      1      |

### Factor de uso

U = 1/4 + 2/5 + 5/20 = 0,9 <= 3(2^(1/3) - 1) = 0,78 -> No Cumple por factor de uso

### Tiempo de respuesta

## Sistema 2

| Tarea | C   | T = D | Prioridades |
| :---- | :-- | :---: | :---------: |
| T1    | 1   |   6   |      3      |
| T2    | 2   |  10   |      2      |
| T3    | 2   |  18   |      1      |

## Sistema 3

| Tarea | C   | T = D | Prioridades |
| :---- | :-- | :---: | :---------: |
| T1    | 1   |   8   |      4      |
| T2    | 3   |  15   |      3      |
| T3    | 4   |  20   |      2      |
| T4    | 6   |  22   |      1      |

### Factor de uso

U = 1/8 + 3/15 + 4/20 + 6/22 = 0,797 <= 4(2^(1/4) - 1) = 0,757 -> No Cumple por factor de uso

## Sistema 4

| Tarea |  C  | T = D | Prioridades |
| :---- | :-: | :---: | :---------: |
| T1    | 0,5 |   4   |      4      |
| T2    |  1  |   5   |      3      |
| T3    |  2  |  10   |      2      |
| T4    |  9  |  24   |      1      |
