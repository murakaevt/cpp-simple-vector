# cpp-simple-vector
Финальный проект: собственный контейнер вектор
Сильно упрощённый аналог стандартного контейнера vector, со сходной структурой и функционалом.
В основе SimpleVector — массив в динамической памяти. 
Сам контейнер хранит лишь адрес начала массива, а также информацию о его текущем размере и вместимости:
Размер — количество элементов, содержащееся в массиве;
Вместимость — максимальное количество элементов, которые могут поместиться в массив.
SimpleVector имеет:
возможность сравнивать содержимое векторов, вставлять и удалять элементы;
пользовательский конструктор копирования;
пользовательский оператор присваивания;
метод Reserve - когда пользователь заранее знает хотя бы приблизительное количество элементов в векторе, reserve сразу выделяет нужное количество памяти. 
При добавлении новых элементов в вектор копирование будет происходить или значительно реже или совсем не будет;
Поддерживает перемещение;
Запускается в IDE.
C++ v17.
