import sys

from collections import namedtuple

from tqdm import tqdm

import pandas as pd

from validate import validate

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


def optimize(data):
    results = {}
    requests = pd.DataFrame(
        data.requests,
        columns=['R_v', 'R_e', 'R_n'])

    endpoint_dc_latency = [
        [index, L_D]
        for index, (L_D, _, _)in enumerate(data.endpoints)]

    endpoint_dc_latency = pd.DataFrame(
        endpoint_dc_latency,
        columns=['R_e', 'L_D'])

    # Merge requests by DC latency to endpoint
    requests = pd.merge(
        left=requests,
        right=endpoint_dc_latency,
        left_on='R_e',
        right_on='R_e',
        how='left')
    assert requests.notnull().all().all(), (
        "None found in request %s" % requests)

    # Sort requests by R_n and L_D
    requests['R_n_L_D'] = requests['R_n'] * requests['L_D']
    requests = requests.sort_values(['R_n_L_D'], ascending=False)

    pbar = tqdm(total=requests.shape[0])

    for _, (R_v, R_e, R_n, L_D, _) in requests.iterrows():
        target_c = -1
        L = L_D
        _, _, connected_caches = data.endpoints[R_e]

        # Objective function
        for c, L_e in connected_caches.items():
            if L_e > L:
                continue

            if L_e == L:
                continue

            # If video R_v not in cache server
            if R_v not in results.get(c, set()):
                capacity = data.X
                for v in results.get(c, set()):
                    capacity -= data.S[v]
                    if capacity < 0:
                        break

                if capacity - data.S[R_v] < 0:
                    continue

            L = L_e
            target_c = c

        if target_c >= 0:
            results.setdefault(target_c, set()).add(R_v)

        pbar.update(1)

    return results


def export(filename, output):
    with open(filename, 'w+') as f:
        f.write('%s\n' % len(output))
        for k, v in output.items():
            v = list(map(str, v))
            f.write('%s %s\n' % (k, ' '.join(v)))


def main():
    filename = sys.argv[1]
    output_filename = sys.argv[2]
    data = load_file(filename)
    output = optimize(data)
    validate(data, output)
    export(output_filename, output)

main()
