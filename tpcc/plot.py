import matplotlib.pyplot as plt
import pandas as pd

plt.style.use('tableau-colorblind10')

columns = [
"SELECT 1;",
"CREATE INDEX IF NOT EXISTS idx_customer_name ON customer (c_w_id, c_d_id, c_last, c_first);",
"CREATE INDEX IF NOT EXISTS idx_order ON oorder (o_w_id, o_d_id, o_c_id, o_id);",
"CREATE INDEX IF NOT EXISTS garbage_1 ON oorder (o_w_id, o_carrier_id, o_ol_cnt);",
"CREATE INDEX IF NOT EXISTS garbage_2 ON foo (a);",
"CREATE INDEX IF NOT EXISTS garbage_3 ON stock (s_w_id, s_i_id);",
"CREATE INDEX IF NOT EXISTS garbage_4 ON district (d_w_id, d_id);",
"CREATE INDEX IF NOT EXISTS garbage_5 ON warehouse (w_name);",
"CREATE INDEX IF NOT EXISTS garbage_6 ON item (i_price, i_data);",
"CREATE INDEX IF NOT EXISTS garbage_7 ON history (h_c_id, h_c_d_id, h_c_w_id);",
"fake",
]


for i in [1,2,3]:
  df = pd.read_csv(f'out{i}.txt', header=None, names=columns)
  df.drop(columns=['fake'], inplace=True)
  print(df)

  df.plot(figsize=(16,12), linewidth=3, fontsize=16)
  plt.title(f'TPC-C: first index built, trial {i}', fontsize=20)
  plt.xlabel('Iteration', fontsize=16)
  plt.ylabel('Probability', fontsize=16)
  plt.savefig(f'fig{i}.png', bbox_inches='tight')
  plt.close()

#plt.show()
