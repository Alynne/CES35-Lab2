# CES35-Lab2

Esse repositório mantem o código referente à atividade de Laboratório 2 da matéria CES-35.

## Integrantes

* Alynne Alencar
* Felipe Coimbra
* Luis Cláudio Magalhães

## Instruções de clone

Clone o repositorio com
```sh
git clone https://github.com/Alynne/CES35-Lab2.git
```
ou
```sh
git clone git@github.com:Alynne/CES35-Lab2.git
```

Utilizamos a framework **GTest** para alguns testes de unidade. Você precisa adquiri-la também por meio de 
```sh
git submodule update --init
```

## Instruções de compilação

O projeto foi feito para ser portavelmente compilável em:
* Mac OS X - 10.15.7
* Ubuntu 18.04 LTS

Tendo sido testado com compiladores
* apple-clang 11.0.3
* g++ 7.5.0

Utilizamos CMake para gerenciar a compilação. Para compilar:

1. Crie uma basta separada
```sh
mkdir build
```

2. Dentro da pasta, configure o projeto
```sh
cd build/
cmake ..
```

3. Compile o projeto
```sh
make
```

## Instruções para execução

Seguindo as instruções anteriores, você encontrará os executáveis `web-client` e `web-server` em uma pasta `build/bin`.
