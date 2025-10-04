#include <stdio.h>
#include <stdlib.h>
#include <pdfio.h>
#include <glib.h>
#include <glib/poppler.h>
#include <cairo.h>

int main(int argc, const char* argv[])
{
  // pdfio_file_t* pdf = pdfioFileOpen("../../data/000233597294.pdf", 
  //                                   NULL,
  //                                   NULL, 
  //                                   NULL,
  //                                   NULL);
  
  const char* pdf_path = "/Users/christian/export/local/works/doublegsoft.open/pdfool/03.Development/pdfool/data/000233597294.pdf";
  GError *error = NULL;

  // Check if file exists first
  if (!g_file_test(pdf_path, G_FILE_TEST_EXISTS)) {
    fprintf(stderr, "File does not exist: %s\n", pdf_path);
    return EXIT_FAILURE;
  }

  gchar* uri = g_filename_to_uri(pdf_path, NULL, &error);
  if (!uri) {
    fprintf(stderr, "Error converting path to URI: %s\n", error ? error->message : "Unknown error");
    if (error) g_error_free(error);
    return EXIT_FAILURE;
  }

  PopplerDocument *doc = poppler_document_new_from_file(uri, NULL, &error);
  g_free(uri);

  if (!doc) {
    fprintf(stderr, "Error opening PDF: %s\n", error->message);
    g_error_free(error);
    return EXIT_FAILURE;
  }

  int n_pages = poppler_document_get_n_pages(doc);
  if (n_pages == 0) {
    fprintf(stderr, "The document has no pages.\n");
    g_object_unref(doc);
    return EXIT_FAILURE;
  }

  printf("Found %d page(s). Converting…\n", n_pages);

  for (int i = 0; i < n_pages; ++i) {
    PopplerPage* page = poppler_document_get_page(doc, i);
    if (!page) {
      fprintf(stderr, "Could not load page %d\n", i + 1);
      continue;
    }

    /* Build output filename: page_1.png, page_2.png, … */
    gchar out_file[256];
    g_snprintf(out_file, sizeof(out_file), "page_%d.png", i + 1);

    /* Get page dimensions and calculate size for 300 DPI */
    double width, height;
    poppler_page_get_size(page, &width, &height);
    
    int pixel_width = (int)(width * 300 / 72.0);
    int pixel_height = (int)(height * 300 / 72.0);
    
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, 
                                                          pixel_width, pixel_height);
    cairo_t* cr = cairo_create(surface);
    
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); 
    cairo_paint(cr);
    
    double scale_x = (double)pixel_width / width;
    double scale_y = (double)pixel_height / height;
    cairo_scale(cr, scale_x, scale_y);
    
    /* Render the page */
    poppler_page_render(page, cr);
    
    /* Write original full page */
    cairo_status_t status = cairo_surface_write_to_png(surface, out_file);
    
    if (status == CAIRO_STATUS_SUCCESS) {
      printf("✓ Saved %s (%dx%d pixels)\n", out_file, pixel_width, pixel_height);
    } else {
      fprintf(stderr, "Error saving page %d: %s\n", i + 1, 
              cairo_status_to_string(status));
    }

    /* Create cropped version */
    int crop_x = 120, crop_y = 60, crop_w = 200, crop_h = 150;
    
    /* Validate crop dimensions */
    if (crop_x + crop_w > pixel_width) crop_w = pixel_width - crop_x;
    if (crop_y + crop_h > pixel_height) crop_h = pixel_height - crop_y;
    if (crop_w <= 0 || crop_h <= 0) {
      fprintf(stderr, "Invalid crop dimensions for page %d\n", i + 1);
      cairo_destroy(cr);
      cairo_surface_destroy(surface);
      g_object_unref(page);
      continue;
    }

    cairo_surface_t* cropped = cairo_image_surface_create(CAIRO_FORMAT_RGB24, 
                                                          crop_w * scale_x, 
                                                          crop_h * scale_y);
    
    if (cairo_surface_status(cropped) != CAIRO_STATUS_SUCCESS) {
      fprintf(stderr, "Error creating cropped surface for page %d\n", i + 1);
      cairo_surface_destroy(cropped);
      cairo_destroy(cr);
      cairo_surface_destroy(surface);
      g_object_unref(page);
      continue;
    }

    /* Create context for cropped surface */
    cairo_t* cr_crop = cairo_create(cropped);

    cairo_set_source_rgb(cr_crop, 1.0, 1.0, 1.0);
    cairo_paint(cr_crop);
    

    cairo_set_source_surface(cr_crop, surface, -crop_x * scale_x, -crop_y * scale_y);
    cairo_paint(cr_crop);

    gchar cropped_file[256];
    g_snprintf(cropped_file, sizeof(cropped_file), "cropped_page_%d.png", i + 1);
    cairo_status_t crop_status = cairo_surface_write_to_png(cropped, cropped_file);
    
    if (crop_status == CAIRO_STATUS_SUCCESS) {
      printf("✓ Saved cropped %s (%dx%d pixels)\n", cropped_file, crop_w, crop_h);
    } else {
      fprintf(stderr, "Error saving cropped page %d: %s\n", i + 1, 
              cairo_status_to_string(crop_status));
    }
    
    /* Clean up cropped resources */
    cairo_destroy(cr_crop);
    cairo_surface_destroy(cropped);
    
    /* Clean up original resources */
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    g_object_unref(page); 
  }

  g_object_unref(doc);
  return 0;
}