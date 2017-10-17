#include "postgresql.h"

class Pessoa : Table<Pessoa>
{
public:
    int id;
    string nome;
    int idade;
    Pessoa() {
        column(id, "id", PrimaryKey);
        column(nome, "nome");
        column(idade, "idade");
    }
};


template<typename T>
T func(string){ return T{}; }



int main()
{
    auto i = func<vector<Pessoa>>(string{});

    auto persist = PostgreSQL::getInstance("dbname=printnow user=postgres password=postgres");

    Pessoa cara;
    cara.id=2;
    cara.nome="Fulano";
    cara.idade=45;
    persist->insert(cara);

    Pessoa&& p = persist->find<Pessoa>(2);
    cout << "Hello " << p.nome << ", idade: " << p.idade << endl;

    cara.nome = "Cicrano";
    cara.idade = 30;
    persist->update(cara, 2);

    vector<Pessoa>&& list = persist->find_list<vector<Pessoa>>("id > 0");
//    for(Pessoa& pessoa: list)
//        cout << "Hello " << pessoa.nome << ", idade: " << pessoa.idade << endl;

    persist->remove(p);

    return 0;
}
