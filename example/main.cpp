#include "storage/postgresql.h"
#include <list>
#include <thread>
#include <d3util/stacktrace.h>
#include <mor/mor.h>

using namespace storage;
struct Avo
{
REFLECTABLE(
    (int) id,
    (string) nome
)
};
ANNOTATIONS_ENTITY(Avo) = {Entity("avo")};
ANNOTATIONS_FIELDS(Avo) = {
    {"id",  {PrimaryKey(), Type("SERIAL")}}
};

struct Pai{
REFLECTABLE(
    (int) id,
    (string) nome,
    (Avo) avo
)
};
ANNOTATIONS_ENTITY(Pai) = {Entity("pai")};
ANNOTATIONS_FIELDS(Pai) = {
    {"id",  {PrimaryKey(), Type("SERIAL")}},
    {"avo", {Reference("avo", "id")}}
};

struct Pessoa
{
REFLECTABLE(
    (int) id,
    (string) nome,
    (int) idade,
    (chrono::time_point<chrono::system_clock>) data,
    (Pai) parente
)
};
ANNOTATIONS_ENTITY(Pessoa) = {Entity("pessoa")};
ANNOTATIONS_FIELDS(Pessoa) = {
    {"id",  {PrimaryKey(), Type("SERIAL")}},
    {"parente", {Reference("pai", "id")}}
};

int main()
{
    ::signal(SIGSEGV, &backtrace_signal_handler);
    ::signal(SIGABRT, &backtrace_signal_handler);
    print_stacktrace_previous_run_crash();

    PostgreSQL db;
    db.connection("host=localhost dbname=postgres user=postgres password=postgres");
    db.create<Avo>();
    db.create<Pai>();
    db.create<Pessoa>();

    Pessoa pessoa;
    pessoa.nome = "Silva Siqueira";
    pessoa.idade = 32;
    pessoa.parente.nome = "PaiDela";

    db.insert(pessoa.parente);
    db.insert(pessoa);

    auto query = db.select<Pessoa>();
    query.where().eq("pessoa.idade", 32)
            .and_().like("pessoa.nome", "%Silv%")
            .and_().like("parente.nome", "%iDel%");
    Pessoa localizada = query;
    cout << localizada.nome << " tem " <<localizada.idade
         << " id: "<< localizada.id
         << " pai: " << localizada.parente.nome << endl;

    Pessoa outro;
    outro.nome = "Beotrano";
    db.insert(outro);

    Expression exp; exp.not_null("pessoa.nome").and_().eq("pessoa.id!", 0);
    list<Pessoa> vec = db.select<Pessoa>().where(exp);
    for(Pessoa& p: vec)
        cout <<'-' << p.nome << " tem " << p.idade << " id: " << p.id
             << " pai: " << localizada.parente.nome << endl;


    outro.id=0;
    outro.nome = "Outro";
    outro.parente.nome = "OutroPai";
    db.insert(outro.parente);
    db.update(outro).where().eq("id", 1);


    vec = db.select<Pessoa>();
    for(Pessoa& p: vec)
        cout << '+' << p.nome << " tem " << p.idade << " pai: "<<p.parente.id<< " id> " << p.id << std::endl;

    db.exec<Pessoa>("select * from pessoa", [&](Pessoa& p){
        cout << '*' << p.nome << " tem " << p.idade << " pai: "<<p.parente.id<< " id> " << p.id << std::endl;
    }).where().eq("id", 1);


    db.drop<Pessoa>();
    db.drop<Pai>();
    db.drop<Avo>();
    return 0;
}
