<?cs def:map_val(val, map) ?>
  <?cs each:item = map ?>
    <?cs if:val == name(item) ?>
      <?cs var:item ?>
    <?cs /if ?>
  <?cs /each ?>
<?cs /def ?>


<?cs var:Output.appinfo.aname ?>

<?cs if:?Output.bills.0.id ?>

<?cs call:map_val(#1, hdfx) ?>


we can use <?cs var: Output.name ?>, but <?cs call: map_val(xxxx) ?> (/def... !expression)will cause:
csparse.c", line 3756, in call_parse() Missing left paren in call  map_val

so, we use <?cs var:Output.name ?> for consistency.
