package geushmond.funfacts;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Button;
import android.widget.Toast;

import java.util.Random;


public class FunFactsActivity extends Activity {

    private static final String TAG = FunFactsActivity.class.getSimpleName();

    private FactBook facts = new FactBook();
    private ColorWheel colors = new ColorWheel();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_fun_facts);

        final TextView factText = (TextView) findViewById(R.id.factText);
        Button showButton = (Button) findViewById(R.id.showButton);

        final RelativeLayout relative = (RelativeLayout) findViewById(R.id.relative);

        View.OnClickListener listener = new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                factText.setText(facts.getFact());
                relative.setBackgroundColor(colors.getColor());
            }
        };
        showButton.setOnClickListener(listener);

        Log.d(TAG, "Oh god");
    }
}
