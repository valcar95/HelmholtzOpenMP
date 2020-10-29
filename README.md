# HelmholtzOpenMP
Paralelización del código para la solución numérica de la ecuación de Helmholtz

## Perfilamiento ## 

Para el ejercicio de perfilamiento se utilizó la herramienta <a href="https://sourceforge.net/projects/qcachegrindwin/">kcachegrind</a> Los archivos que fueron insumo para kcachegrind se generaron con valgrind mediante el comando: <i>valgrind --tool=callgrind ./jacobi.o</i>
<br>
<br>
<b>Resultados del perfilamiento:</b><br>
<table>
<tr valign="bottom">
<td>
<img src="./img/jacobi2.JPG"/><br>
<i>fig. 1. Call Graph</i>
</td>
<td>
<img src="./img/jacobi3.JPG"/><br>
<i>fig. 2. Callees</i>
</td>
</tr>
</table><br>

Como se puede observar en la <i>fig. 2.</i> el 98.12% de las llamadas se hicieron mientras el algoritmo
se encontraba en la función <i>jacobi</i>. Esta será la función donde concentraremos la mayor parte
del esfuerzo de optimización.

## Optimización ## 
<b>Resultados de ejecución inicial:</b><br><br>
<img src="./img/time1.JPG"/><br>
<i>fig. 3. Resultados de ejecución sin optimización</i>
<br>

<b>Técnica de optimización:</b><br>

La principal optimización se realizó sobre el método jacobi, específicamente en el segundo for anidado dentro del while. Allí se aplicó el patrón tiling con el objetivo de disminuir el número de cache misses al momento de obtener datos de nuestro array bidimensional. El tamaño del tile para las filas se calculó con base en la línea de caché [1], mientras el tamaño tile de las columnas se dejó en 1 (se probaron otros tamaños, pero el mejor resultado fue con un tamaño de 1). Dado que cada iteración del ciclo anidado sumaba sus resultados a una variable compartida, también se utilizó la directiva <i>reduction</i> de OpenMP. Se aplicaron otras optimizaciones sobre otras regiones principalmente con la directiva <i>collapse</i> para mejorar la distribución de las iteraciones en ciclos anidados. Adicionalmente en cada iteración se propone ejecutar dos veces el cálculo (con índices i y i+1) con el objetivo de hacer un mejor uso de las unidades aritméticas que pueda tener cada procesador [2].  



<b>Referencias</b>

<ul>
    <li>
        <a href="https://www.sciencedirect.com/topics/computer-science/loop-tiling">
            <i>
            [1]	A. Duran and L. Meadows, “A Many-Core Implementation of the Direct N-Body Problem,” in High Performance Parallelism Pearls: Multicore and Many-core Programming Approaches, Elsevier Inc., 2015, pp. 159–174.
            </i>
        </a>
    </li>
    <li>
        <a href="https://m-sp.org/downloads/pp_report_openmp.pdf">
            <i>
            [2]	M. Springer, “Practical Parallel Computing, Assignment 1-Diffusion.”
            </i>
        </a>
    </li>
</ul>
