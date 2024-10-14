import pandas as pd
import plotly.express as px 
import sys

cols=['ts', 'r', 's']
df = pd.read_csv(
    sys.argv[1] if len(sys.argv) > 1 else 'data.csv',
    names = cols)

fig = px.line(df, x='ts', y=['r', 's'])

fig.show()
