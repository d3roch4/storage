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
    int id2=0;
    string nome;
    int idade;
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

    cout << where(Condition{"col1"} > 87, AND, Condition{"col2"} % "abacate") << endl;
    db.create<Pessoa>();



    db.drop<Pessoa>();
    return 0;
}
