#include "sqlite.h"
#include "postgresql.h"
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
        field(id, "id", {{"attrib","PK"}, {"type_db","SERIAL"}});
        field(id2, "id2", {{"attrib","PK"}, {"type_db","SERIAL"}});
        field(nome, "nome");
        field(idade, "idade");
        field(data, "data");
    }
    ~Pessoa(){

    }
};


shared_ptr<PostgreSQL> persist;
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
    persist =  PostgreSQL::getInstance("host=localhost dbname=printnow user=postgres password=postgres"); //SQLite::getInstance("teste.db");
    cout << where(Condition{"col1"} > 87, AND, Condition{"col2"} % "abacate") << endl;
    persist->drop<Pessoa>();
    persist->create<Pessoa>();


    insert();
    Pessoa& copia = q.back();

    Pessoa&& p = persist->find<Pessoa>(where(Condition{"nome"} % "Ful" ));
    cout << "Hello " << p.nome << ", id: " <<p.id << ", idade: " << p.idade << endl;

    copia.nome = "Cicrano";
    copia.idade = 30;
    persist->update(copia, "id="+to_string(1));

    copia.id=0;
    copia.nome = "Bertuliano";
    persist->insert(copia);
    copia.id=0;
    copia.nome = "Teotrano";
    persist->insert(copia);

    vector<Pessoa>&& list = persist->find_list<vector<Pessoa>>(where(condition("id", BIGGER_THEN, 0) ));
    for(Pessoa& pessoa: list)
        cout << "Hello " << pessoa.nome << ", idade: " << pessoa.idade << ", data: " << Field<chrono::time_point<std::chrono::system_clock>>(pessoa.data).getValue() << endl;

    persist->remove(p);
    persist->drop<Pessoa>();

    return 0;
}
