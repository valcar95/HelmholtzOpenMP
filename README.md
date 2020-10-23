# HelmholtzOpenMP
Paralelización del código para la solución numérica de la ecuación de Helmholtz

## Perfilamiento ## 

Para el ejercicio de perfilamiento se utilizó la herramienta <a href="https://sourceforge.net/projects/qcachegrindwin/">kcachegrind</a> Los archivos que fueron insumo para kcachegrind se generaron con valgrind mediante el comando: <i>valgrind --tool=callgrind ./jacobi.o</i>
<br>
<br>
<b>Resultados del perfilamiento:</b><br>
<table>
<tr>
<td>
<img src="./img/jacobi2.JPG"/><br>
<i>fig. 1: Call Graph</i>
</td>
<td>
<img src="./img/jacobi3.JPG"/><br>
<i>fig. 2: Callees</i>
</td>
</tr>
</table>