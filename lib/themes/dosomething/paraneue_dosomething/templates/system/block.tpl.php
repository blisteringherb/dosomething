<?php
/**
 * Returns the HTML for a block.
 * @see https://drupal.org/node/1728246
 */
?>

<?php if ($clean_output): ?>

  <?php print $content ?>

<?php else: ?>

  <div id="<?php print $block_html_id; ?>" class="<?php print $classes; ?>"<?php print $attributes; ?>>

    <?php print render($title_prefix); ?>
  <?php if ($block->subject): ?>
    <h2<?php print $title_attributes; ?>><?php print $block->subject ?></h2>
  <?php endif;?>
    <?php print render($title_suffix); ?>

    <div class="content"<?php print $content_attributes; ?>>
      <?php print $content ?>
    </div>
  </div>

<?php endif; ?>
