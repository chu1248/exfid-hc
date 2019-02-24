import sys
from os.path import join, dirname, basename, exists

from collections import namedtuple

import pandas as pd

Data = namedtuple('Data', [
    'V',
    'E',
    'R',
    'C',
    'X',
    'S',
    'endpoints',
    'requests',
])


def load_file(filename):
    with open(filename, 'r') as f:
        V, E, R, C, X = list(map(int, f.readline().split(' ')))
        S = list(map(int, f.readline().split(' ')))
        endpoints = []

        for i in range(0, E):
            L_D, K = [
                int(x)
                for x in f.readline().split(' ')]
            cache_servers = []

            for k in range(0, K):
                c, L_c = list(map(int, f.readline().split(' ')))
                cache_servers.append((c, L_c))

            endpoints.append((L_D, K, dict(cache_servers)))

        requests = []
        for i in range(0, R):
            R_v, R_e, R_n = list(
                map(int, f.readline().split(' ')))
            requests.append((R_v, R_e, R_n))

        return Data(
            V=V,
            E=E,
            R=R,
            C=C,
            X=X,
            S=S,
            endpoints=endpoints,
            requests=requests)


def load_output(data, filename):
    with open(filename, 'r') as f:
        N = int(f.readline())
        servers = []
        for index in range(0, N):
            c, *v = list(map(int, f.readline().split(' ')))

            assert 0 <= c < data.C, (
                "Server index %s exceeds the total "
                "cache servers %s on line %s") % (
                    c, data.C, index)

            total_storage = sum([
                data.S[i] for i in v])
            assert 0 <= total_storage <= data.X, (
                "Total storage %s exceeds the capacity "
                "of the cache server %s on line %s") % (
                    total_storage, data.X, index)

            servers.append((c, v))

        servers = dict(servers)
        return servers


def validate(data, servers):
    servers = dict(servers)
    total_score = 0
    total_requests = 0

    for R_v, R_e, R_n in data.requests:
        L_D, K, cache_servers = data.endpoints[R_e]
        L = L_D

        for c, L_e in cache_servers.items():
            if c in servers and R_v in servers[c]:
                L = min(L, L_e)

        score = R_n * (L_D - L)
        print(('{R_n} of request {R_v} from endpoint {R_e} '
               'has score {score}').format(
                   R_n=R_n,
                   R_v=R_v,
                   R_e=R_e,
                   score=score))
        total_requests += R_n
        total_score += score

    total_score = total_score / total_requests * 1000
    print('Total score: %s' % total_score)
    return total_score


def main():
    filename = sys.argv[1]
    output_filename = sys.argv[2]
    data = load_file(filename)
    output = load_output(data, output_filename)
    total_score = validate(data, output)

    score_stats = join(dirname(output_filename), 'final.h5')
    if exists(score_stats):
        final_result = pd.read_hdf(score_stats)
    else:
        final_result = pd.Series()

    final_result[basename(filename)] = total_score
    print('Exporting to the score to %s', score_stats)
    final_result.to_hdf(score_stats, '/scores')


if __name__ == '__main__':
    main()

