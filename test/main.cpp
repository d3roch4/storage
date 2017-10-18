#include "postgresql.h"

class Pessoa : private Table<Pessoa>
{
public:
    int id;
    string nome;
    int idade;
    chrono::time_point<chrono::system_clock> data;
    Pessoa() {
        column(id, "id", PrimaryKey);
        column(nome, "nome");
        column(idade, "idade");
        column(data, "data");
    }
};


template<typename T>
T func(string){ return T{}; }



int main()
{
    auto i = func<vector<Pessoa>>(string{});

    auto persist = PostgreSQL::getInstance("dbname=printnow user=postgres password=postgres");
    persist->drop<Pessoa>();
    persist->create<Pessoa>();

    Pessoa cara;
    cara.id=2;
    cara.nome="Fulano";
    cara.idade=45;
    cara.data = chrono::system_clock::now();
    persist->insert(cara);

    Pessoa&& p = persist->find<Pessoa>(2);
    cout << "Hello " << p.nome << ", idade: " << p.idade << endl;

    cara.nome = "Cicrano";
    cara.idade = 30;
    persist->update(cara, 2);

    vector<Pessoa>&& list = persist->find_list<vector<Pessoa>>("id > 0");
    for(Pessoa& pessoa: list)
        cout << "Hello " << pessoa.nome << ", idade: " << pessoa.idade << ", data: " << Column<chrono::time_point<std::chrono::system_clock>>(pessoa.data).to_string() << endl;

    persist->remove(p);
    persist->drop<Pessoa>();

    return 0;
}
