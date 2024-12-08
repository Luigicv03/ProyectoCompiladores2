#include <iostream>
#include <sstream>
#include <cmath>
#include <map>
#include <string>
#include <vector>
#include <cctype>

using namespace std;

// Definir tipos de tokens
enum class TokenType {
    Number, Variable, Operator, Function, ParenOpen, ParenClose, Equal, End, Invalid
};

// Estructura para almacenar un token
struct Token {
    TokenType type;
    string value;

    Token(TokenType type, string value = "") : type(type), value(value) {}
};

// Mapa de variables
map<string, double> variables;

// Prototipos
vector<Token> lex(const string& input);
double evaluate(const vector<Token>& tokens, size_t& idx);
double parseExpression(const vector<Token>& tokens, size_t& idx);
double parseTerm(const vector<Token>& tokens, size_t& idx);  // Modificado para incluir potencia
double parseFactor(const vector<Token>& tokens, size_t& idx);
double parseFunction(const vector<Token>& tokens, size_t& idx);
double applyOperation(double left, double right, const string& op);
double root(double value);

// Función principal
int main() {
    string line;
    while (true) {
        cout << "Ingrese una expresion o 'exit' para terminar: ";
        getline(cin, line);
        
        if (line == "exit") break;

        vector<Token> tokens = lex(line);
        
        if (tokens.empty()) {
            cout << "Entrada invalida.\n";
            continue;
        }

        size_t idx = 0;
        double result = evaluate(tokens, idx);
        cout << "Resultado: " << result << endl;
    }

    return 0;
}

// Función para hacer el análisis léxico
vector<Token> lex(const string& input) {
    vector<Token> tokens;
    stringstream ss(input);
    char ch;
    
    while (ss >> noskipws >> ch) {
        if (isdigit(ch) || ch == '.') {
            string num(1, ch);
            while (ss.peek() != EOF && (isdigit(ss.peek()) || ss.peek() == '.')) {
                ss >> ch;
                num += ch;
            }
            tokens.push_back(Token(TokenType::Number, num));
        } else if (isalpha(ch)) {
            string var(1, ch);
            while (ss.peek() != EOF && isalpha(ss.peek())) {
                ss >> ch;
                var += ch;
            }
            if (var == "root") {
                tokens.push_back(Token(TokenType::Function, var));
            } else {
                tokens.push_back(Token(TokenType::Variable, var));
            }
        } else if (ch == '=') {
            tokens.push_back(Token(TokenType::Equal, "="));
        } else if (ch == '+') {
            tokens.push_back(Token(TokenType::Operator, "+"));
        } else if (ch == '-') {
            tokens.push_back(Token(TokenType::Operator, "-"));
        } else if (ch == '*') {
            tokens.push_back(Token(TokenType::Operator, "*"));
        } else if (ch == '/') {
            tokens.push_back(Token(TokenType::Operator, "/"));
        } else if (ch == '^') {
            tokens.push_back(Token(TokenType::Operator, "^"));  // Reconocer el operador de potencia
        } else if (ch == '(') {
            tokens.push_back(Token(TokenType::ParenOpen, "("));
        } else if (ch == ')') {
            tokens.push_back(Token(TokenType::ParenClose, ")"));
        } else if (isspace(ch)) {
            continue;
        } else {
            tokens.push_back(Token(TokenType::Invalid, string(1, ch)));
        }
    }

    tokens.push_back(Token(TokenType::End));
    return tokens;
}

// Función para evaluar los tokens
double evaluate(const vector<Token>& tokens, size_t& idx) {
    if (tokens[idx].type == TokenType::Variable && tokens[idx + 1].type == TokenType::Equal) {
        // Caso de asignación
        string var = tokens[idx].value;
        idx += 2;  // Saltar la variable y el '='
        double value = parseExpression(tokens, idx);
        variables[var] = value;  // Asignar el valor a la variable
        return value;
    }

    return parseExpression(tokens, idx);
}

// Función para analizar expresiones
double parseExpression(const vector<Token>& tokens, size_t& idx) {
    double result = parseTerm(tokens, idx);
    
    while (tokens[idx].type == TokenType::Operator && 
           (tokens[idx].value == "+" || tokens[idx].value == "-")) {
        string op = tokens[idx].value;
        idx++;
        double term = parseTerm(tokens, idx);
        result = applyOperation(result, term, op);
    }
    
    return result;
}

// Función para analizar términos (multiplicación, división y potencia)
double parseTerm(const vector<Token>& tokens, size_t& idx) {
    double result = parseFactor(tokens, idx);
    
    while (tokens[idx].type == TokenType::Operator && 
           (tokens[idx].value == "*" || tokens[idx].value == "/" || tokens[idx].value == "^")) {  // Añadir el ^ aquí
        string op = tokens[idx].value;
        idx++;
        double factor = parseFactor(tokens, idx);
        result = applyOperation(result, factor, op);
    }
    
    return result;
}

// Función para analizar factores (números, variables, paréntesis)
double parseFactor(const vector<Token>& tokens, size_t& idx) {
    double result = 0;
    
    if (tokens[idx].type == TokenType::Number) {
        result = stod(tokens[idx].value);
        idx++;
    } else if (tokens[idx].type == TokenType::Variable) {
        string var = tokens[idx].value;
        // Comprobar si la variable está definida
        if (variables.find(var) != variables.end()) {
            result = variables[var];
        } else {
            cout << "Error: variable " << var << " no definida.\n";
            exit(1);
        }
        idx++;
    } else if (tokens[idx].type == TokenType::ParenOpen) {
        idx++;
        result = parseExpression(tokens, idx);
        if (tokens[idx].type != TokenType::ParenClose) {
            cout << "Error: parentesis no cerrado.\n";
            exit(1);
        }
        idx++;
    } else if (tokens[idx].type == TokenType::Function) {
        string func = tokens[idx].value;
        idx++;
        if (func == "root") {
            if (tokens[idx].type != TokenType::ParenOpen) {
                cout << "Error: se esperaba '('.\n";
                exit(1);
            }
            idx++;
            result = parseExpression(tokens, idx);
            if (tokens[idx].type != TokenType::ParenClose) {
                cout << "Error: parentesis no cerrado.\n";
                exit(1);
            }
            idx++;
            result = root(result);
        }
    }

    return result;
}

// Función para aplicar una operación entre dos operandos
double applyOperation(double left, double right, const string& op) {
    if (op == "+") return left + right;
    if (op == "-") return left - right;
    if (op == "*") return left * right;
    if (op == "/") return left / right;
    if (op == "^") return pow(left, right);  // Aplicar la potencia
    return 0;
}

// Función para calcular la raíz cuadrada
double root(double value) {
    if (value < 0) {
        cout << "Error: raíz cuadrada de un número negativo.\n";
        exit(1);
    }
    return sqrt(value);
}