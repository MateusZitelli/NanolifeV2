from pylab import *
import gen_runner
def similarity(ar1, ar2):
        s = .0
        if(len(ar1) > len(ar2)): ar1, ar2 = ar2, ar1
        for i, j in enumerate(ar1):
                if j == ar2[i]: s += 1
        return s / float(len(ar1))
        
def get_important_part(gen):
        for i in range(10):
                times = gen_runner.run(gen)
                if sum([1 for b, k in enumerate(times) if k > 0]): break
        return [j[b] for b, k in enumerate(times) if k > 0]
data = [[eval("0x" + k) for k in i.split('#')[3][1:].split(',')] for i in open('genetic_tree.data', 'r').read().split('\n')[:-1]]
last = 0
lista = []
for i, j in enumerate(data[::1]):
        lista.append(len(get_important_part(j)))
        print get_important_part(j)
plot(lista)
show()

