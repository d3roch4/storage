#include "sqlite.h"
#include <list>

class Pessoa : private Entity<Pessoa>
{
public:
    int id=0;
    int id2=0;
    string nome;
    int idade;
    chrono::time_point<chrono::system_clock> data;
    Pessoa() {
        column(id, "id", PrimaryKey, "SERIAL");
        column(id2, "id2", PrimaryKey, "SERIAL");
        column(nome, "nome");
        column(idade, "idade");
        column(data, "data");
    }
    ~Pessoa(){

    }
};


auto persist = SQLite::getInstance("teste.db");
list<Pessoa> q;

void func(Pessoa& copia){
    q.emplace_back(copia);
}

void insert(){
    Pessoa cara;
    cara.id2=56;
    cara.nome="Fulano";
    cara.idade=45;
    cara.data = chrono::system_clock::now();
    persist->insert(cara);
    cout << "inserted with id: " << cara.id << " id2: "<<cara.id2 << endl;

    func(cara);

    Pessoa& outra = q.back();
    outra.id=5;
}

int main()
{
    cout << where(Condition{"col1"} > 87, AND, Condition{"col2"} % "abacate") << endl;
    persist->drop<Pessoa>();
    persist->create<Pessoa>();

    insert();
    Pessoa& copia = q.back();

    persist->insert(copia);
    Pessoa&& p = persist->find<Pessoa>(where(Condition{"id"} > 0 ));
    cout << "Hello " << p.nome << ", id: " <<p.id << ", idade: " << p.idade << endl;

    copia.nome = "Cicrano";
    copia.idade = 30;
    persist->update(copia, "id="+to_string(1));

    vector<Pessoa>&& list = persist->find_list<vector<Pessoa>>(where(condition("id", BIGGER_THEN, 2) ));
    for(Pessoa& pessoa: list)
        cout << "Hello " << pessoa.nome << ", idade: " << pessoa.idade << ", data: " << Column<chrono::time_point<std::chrono::system_clock>>(pessoa.data).getValue() << endl;

    persist->remove(p);
    persist->drop<Pessoa>();

    return 0;
}
