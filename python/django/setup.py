import setuptools

from ci.pkgenv import PROJECT_NAME, VERSION

with open('ptc_deco/requirements.txt') as f:
    requirements = [s for s in (x.strip() for x in f.read().split('\n')) if s]

print(requirements)

setuptools.setup(
    name=PROJECT_NAME,  # Replace with your own username
    version=VERSION,
    description='PTC Deco backend',
    url='https://bitbucket.org/ptcdeco/ptc-backend',
    packages=setuptools.find_packages(exclude=('ci', 'forks')),
    classifiers=[
        'Programming Language :: Python :: 3',
        'Operating System :: OS Independent',
    ],
    package_data={
        '': [
            'api/sql/**/*',
            'api/sql/functions/**/*',
            'api/api_gql/mutations/documents/estimate_documents/wordgen/xml/*',
        ],
    },
    python_requires='>=3.6',
    install_requires=requirements,
)
