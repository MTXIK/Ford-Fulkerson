#include <iostream>
#include <fstream>
#include <vector>
#include <queue>

using namespace std;

// Структура для описания ребра графа
struct Edge {
    int to;              // Конечная вершина ребра
    int capacity;        // Пропускная способность ребра
    int flow;            // Текущий поток через ребро
    Edge* reverse;       // Указатель на обратное ребро

    // Конструктор для инициализации ребра
    Edge(int to, int capacity) : to(to), capacity(capacity), flow(0), reverse(nullptr) {}
};

// Функция для чтения графа из бинарного файла
bool readGraph(const string& filename, vector<vector<Edge*> >& adjList, int& source, int& sink) {
    ifstream inputFile(filename, ios::binary); // Открываем файл в бинарном режиме
    if (!inputFile.is_open()) {
        cerr << "Error: Cannot open input file." << endl;
        return false;
    }

    int16_t numVertices;
    inputFile.read(reinterpret_cast<char*>(&numVertices), sizeof(numVertices)); // Считываем количество вершин
    adjList.resize(numVertices); // Создаем вектор списков смежности

    while (!inputFile.eof()) {
        int16_t from, to, capacity;
        inputFile.read(reinterpret_cast<char*>(&from), sizeof(from));        // Начальная вершина ребра
        inputFile.read(reinterpret_cast<char*>(&to), sizeof(to));            // Конечная вершина ребра
        inputFile.read(reinterpret_cast<char*>(&capacity), sizeof(capacity)); // Пропускная способность

        if (inputFile.eof()) break; // Проверяем на конец файла

        // Создаем два ребра: прямое и обратное
        Edge* forward = new Edge(to, capacity); // Прямое ребро с заданной пропускной способностью
        Edge* backward = new Edge(from, 0);     // Обратное ребро с нулевой пропускной способностью

        forward->reverse = backward; // Устанавливаем связь между прямым и обратным ребром
        backward->reverse = forward;

        adjList[from].push_back(forward); // Добавляем прямое ребро в список смежности
        adjList[to].push_back(backward);  // Добавляем обратное ребро
    }

    inputFile.close();

    // Пользователь вводит номера начальной (исток) и конечной (сток) вершин
    cout << "Enter source vertex: ";
    cin >> source;
    cout << "Enter sink vertex: ";
    cin >> sink;

    return true;
}

// BFS для поиска пути в остаточной сети
bool bfs(vector<vector<Edge*> >& adjList, vector<Edge*>& parent, int source, int sink) {
    int numVertices = adjList.size(); // Количество вершин графа
    parent.assign(numVertices, nullptr); // Инициализируем массив для восстановления пути
    vector<bool> visited(numVertices, false); // Массив для отслеживания посещенных вершин
    queue<int> q; // Очередь для BFS

    // Начинаем поиск с исходной вершины
    q.push(source);
    visited[source] = true;

    while (!q.empty()) {
        int current = q.front(); // Извлекаем текущую вершину
        q.pop();

        // Проходим по всем рёбрам текущей вершины
        for (Edge* edge : adjList[current]) {
            // Проверяем, можно ли пройти по ребру
            if (!visited[edge->to] && edge->capacity > edge->flow) {
                parent[edge->to] = edge; // Запоминаем ребро, через которое пришли
                visited[edge->to] = true; // Отмечаем вершину как посещенную
                q.push(edge->to); // Добавляем вершину в очередь

                // Если дошли до стока, заканчиваем поиск
                if (edge->to == sink) {
                    return true;
                }
            }
        }
    }

    // Пути до стока нет
    return false;
}

// Алгоритм Форда-Фалкерсона для поиска максимального потока
int fordFulkerson(vector<vector<Edge*> >& adjList, int source, int sink) {
    int maxFlow = 0; // Начальное значение максимального потока
    vector<Edge*> parent; // Массив для восстановления пути

    // Пока существует путь в остаточной сети
    while (bfs(adjList, parent, source, sink)) {
        int pathFlow = INT32_MAX; // Инициализируем поток на пути как бесконечность

        // Ищем минимальную пропускную способность на найденном пути
        for (int v = sink; v != source; v = parent[v]->reverse->to) {
            pathFlow = min(pathFlow, parent[v]->capacity - parent[v]->flow);
        }

        // Обновляем потоки на рёбрах пути
        for (int v = sink; v != source; v = parent[v]->reverse->to) {
            parent[v]->flow += pathFlow; // Увеличиваем поток в прямом ребре
            parent[v]->reverse->flow -= pathFlow; // Уменьшаем поток в обратном ребре
        }

        maxFlow += pathFlow; // Увеличиваем общий максимальный поток
    }

    return maxFlow; // Возвращаем значение максимального потока
}

// Функция для записи результата в текстовый файл, обходя граф в ширину
void writeOutputBFS(const string& filename, int maxFlow, const vector<vector<Edge*> >& adjList, int source, int sink) {
    ofstream outputFile(filename); // Открываем файл для записи

    if (!outputFile.is_open()) {
        cerr << "Error: Cannot open output file." << endl;
        return;
    }

    // Записываем максимальный поток и его значение
    outputFile << "Value of maximum flow from " << source << " to " << sink
               << " vertices: " << maxFlow << endl;

    // Выполняем BFS для определения порядка обхода вершин
    int numVertices = adjList.size();
    vector<bool> visited(numVertices, false);
    queue<int> q;
    vector<int> bfsOrder;

    // Начинаем BFS с вершины source
    q.push(source);
    visited[source] = true;

    while (!q.empty()) {
        int current = q.front();
        q.pop();
        bfsOrder.push_back(current);

        for (Edge* edge : adjList[current]) {
            if (!visited[edge->to] && edge->capacity > edge->flow) { // Остаточная пропускная способность
                visited[edge->to] = true;
                q.push(edge->to);
            }
        }
    }

    // Записываем потоки в порядке BFS
    outputFile << "Flow (traversed in BFS order):" << endl;
    for (int vertex : bfsOrder) {
        for (Edge* edge : adjList[vertex]) {
            if (edge->flow > 0) { // Сохраняем только те рёбра, через которые идёт поток
                outputFile << "(" << vertex << ", " << edge->to << ", " << edge->flow << ")" << endl;
            }
        }
    }

    outputFile.close(); // Закрываем файл
}

void writeOutput(const string& filename, int maxFlow, const vector<vector<Edge*> >& adjList, int source, int sink) {
    ofstream outputFile(filename); // Открываем файл для записи

    if (!outputFile.is_open()) {
        cerr << "Error: Cannot open output file." << endl;
        return;
    }

    // Записываем максимальный поток и его значение
    outputFile << "Value of maximum flow from " << source << " to " << sink
               << " vertices: " << maxFlow << endl;

    // Записываем потоки через рёбра
    outputFile << "Flow:" << endl;
    for (size_t from = 0; from < adjList.size(); ++from) {
        for (Edge* edge : adjList[from]) {
            if (edge->flow > 0) { // Сохраняем только те рёбра, через которые идёт поток
                outputFile << "(" << from << ", " << edge->to << ", " << edge->flow << ")" << endl;
            }
        }
    }

    outputFile.close(); // Закрываем файл
}


// Главная функция программы
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input file> [-o <output file>]" << endl;
        return 1;
    }

    string inputFile = argv[1]; // Имя входного файла
    string outputFile = "output.txt"; // Имя выходного файла по умолчанию

    // Обработка параметра -o для указания имени выходного файла
    for (int i = 2; i < argc; ++i) {
        if (string(argv[i]) == "-o" && i + 1 < argc) {
            outputFile = argv[i + 1];
            break;
        }
    }

    vector<vector<Edge*> > adjList; // Списки смежности графа
    int source, sink; // Исток и сток графа

    // Читаем граф из входного файла
    if (!readGraph(inputFile, adjList, source, sink)) {
        return 1;
    }

    // Выполняем алгоритм Форда-Фалкерсона
    int maxFlow = fordFulkerson(adjList, source, sink);
    
    int option;

    cout << "Choose output option:" << endl;
    cout << "1 - First to last vertex order" << endl;
    cout << "2 - Source to sink vertex order" << endl;
    cout << "Your choice: ";
    cin >> option;
    
    // Запись результата в выходной файл в зависимости от выбора
    if (option == 1){
        writeOutput(outputFile, maxFlow, adjList, source, sink);
    }
    else if (option == 2){
        writeOutputBFS(outputFile, maxFlow, adjList, source, sink);
    }
    else {
        cerr << "Invalid option selected. Please choose 1 or 2." << endl;
        return 1;
    }
    

    // Выводим сообщение о завершении работы программы
    cout << "Max flow calculated and written to " << outputFile << endl;
    
    for(auto & edges : adjList){
            for(auto edge : edges){
                delete edge;
            }
        }
        
    return 0;
}
//g++ -std=c++11 -o two two.cpp
