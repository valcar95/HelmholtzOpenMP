# HelmholtzOpenMP
Paralelización del código para la solución numérica de la ecuación de Helmholtz

## Perfilamiento ## 

Para el ejercicio de perfilamiento se utilizó la herramienta <a href="https://sourceforge.net/projects/qcachegrindwin/">kcachegrind</a> Los archivos que fueron insumo para kcachegrind se generaron con valgrind mediante el comando: <i>valgrind --tool=callgrind ./a.out</i>