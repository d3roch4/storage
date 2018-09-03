#include "storage/postgresql.h"
#include <list>
#include <thread>
#include <d3util/stacktrace.h>

using namespace storage;
using namespace mor;

class Pessoa : public Entity<Pessoa>
{
public:
    int id=0;
    string nome;
    int idade=0;
    chrono::time_point<chrono::system_clock> data;
    Pessoa() {
        field(id, "id", {{"attrib","PK"}, {"type_db","SERIAL"}});
        field(nome, "nome");
        field(idade, "idade");
        field(data, "data");
    }
};



int main()
{
    ::signal(SIGSEGV, &backtrace_signal_handler);
    ::signal(SIGABRT, &backtrace_signal_handler);
    print_stacktrace_previous_run_crash();

    PostgreSQL db;
    db.connection("host=localhost dbname=teste user=postgres password=postgres");
    db.create<Pessoa>();

    Pessoa pessoa;
    pessoa.nome = "Silva Siqueira";
    pessoa.idade = 32;

    db.insert(pessoa);

    auto query = db.select<Pessoa>();
    query.where().eq("idade", 32).or_().like("nome", "%Silv%");
    Pessoa&& localizada = query;
    cout << localizada.nome << " tem " <<localizada.idade<< " id "<< localizada.id << endl;

    Pessoa outro;
    outro.nome = "Beotrano";
    db.insert(outro);

    Expression exp; exp.not_null("pessoa.nome").and_().eq("pessoa.id!", 0);
    vector<Pessoa>&& vec = db.select<Pessoa>().where(exp);
    for(Pessoa& p: vec)
        cout << p.nome << " tem " << p.idade << " id: " << p.id << std::endl;


    outro.id=0;
    outro.nome = "Outro";
    db.update(outro).where().eq("id", 1);


    vec = db.select<Pessoa>();
    for(Pessoa& p: vec)
        cout << p.nome << " tem " << p.idade << " id> " << p.id << std::endl;

    db.drop<Pessoa>();
    return 0;
}
