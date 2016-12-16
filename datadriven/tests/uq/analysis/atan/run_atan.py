'''
Created on Sep 19, 2016

@author: franzefn
'''
from argparse import ArgumentParser
from multiprocessing.process import Process
from uq.analysis.atan.test_atan import run_atan_pce, run_atan_sg

if __name__ == '__main__':
    parallel = False

    parser = ArgumentParser(description='Get a program and run it with input', version='%(prog)s 1.0')
    parser.add_argument('--surrogate', default="both", type=str, help="define which surrogate model should be used (sg, pce)")
    parser.add_argument('--full', default=False, action='store_true', help='run the full model')
    parser.add_argument('--reduced', default=False, action='store_true', help='run the reduced model')
    parser.add_argument('--out', default=False, action='store_true', help='write results to file')
    parser.add_argument('--plot', default=False, action='store_true', help='plot results (1d)')
    parser.add_argument('--parallel', default=False, action='store_true', help='run in parallel')
    args = parser.parse_args()

    scenarions_sg = {'uniform': {'gridType': ["polyBoundary",
                                              "modPoly",
                                              "polyClenshawCurtisBoundary",
                                              "modPolyClenshawCurtis"],
                                 'level': [2],
                                 'refinement': [None,
                                                'simple',
                                                'weighted',
                                                'squared',
                                                'var',
                                                'exp'],
                                 'maxGridPoints': [3000]},
                     'normal': {'gridType': ["polyBoundary",
                                              "modPoly",
                                              "polyClenshawCurtisBoundary",
                                              "modPolyClenshawCurtis"],
                                 'level': [2],
                                 'refinement': [None,
                                                'simple',
                                                'weighted',
                                                'squared',
                                                'var',
                                                'exp'],
                                 'maxGridPoints': [3000]}}

    scenarions_pce = {'uniform': {'sampler': ['fekete', 'leja', 'gauss'],
                                  'expansion': ["full_tensor", "total_degree"],
                                  'max_num_samples': [3000]},
                      'normal': {'sampler': ['fekete', 'leja', 'gauss'],
                                 'expansion': ["full_tensor", "total_degree"],
                                 'max_num_samples': [3000]}}

    processes = []
    if args.surrogate in ["both", "sg"]:
        for model, surrogate in scenarions_sg.items():
            for gridType in surrogate['gridType']:
                for refinement in surrogate['refinement']:
                    maxGridPoints = [surrogate['maxGridPoints'][-1]]
                    if refinement is not None:
                        maxGridPoints = surrogate['maxGridPoints']

                    for maxGridPoint in maxGridPoints:
                        print "-" * 80
                        print "scenario: (%s, %s, %i, %s)" % (model, gridType, maxGridPoint, refinement)
                        print "-" * 80

                        if args.parallel:
                            myargs = (model, gridType, maxGridPoint,
                                      1, False, refinement, args.out)
                            processes.append(Process(target=run_atan_sg, args=myargs))
                        else:
                            run_atan_sg(model,
                                        gridType,
                                        maxGridPoint,
                                        1,
                                        False,
                                        refinement,
                                        args.out)

    if args.surrogate in ["both", "pce"]:
        for model, surrogate in scenarions_pce.items():
            for sampler in surrogate['sampler']:
                for expansion in surrogate["expansion"]:
                    for max_num_samples in surrogate["max_num_samples"]:
                        print "-" * 80
                        print "scenario: (%s, %s, %s, %i)" % (model, sampler, expansion, max_num_samples)
                        print "-" * 80

                        if args.parallel:
                            myargs = (model, sampler, expansion, max_num_samples, args.out)
                            processes.append(Process(target=run_atan_pce, args=myargs))
                        else:
                            run_atan_pce(model, sampler, expansion, max_num_samples, args.out)

    # run applications in parallel if there are any available
    for process in processes:
        process.start()
