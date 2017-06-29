package mooltipass.automatedTest.features;

import org.junit.rules.Verifier;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Trillian;

public class TrillianSteps {

	Trillian trillian= new Trillian(WebDriverFactory.get());

	@When("I login trillian with '(.*)' and '(.*)'")
	public void login(String username,String password){
		trillian.enterEmail(username);
		trillian.enterPassword(password);
		trillian.submit();
		
	}

	@Then("I should be logged in trillian")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",trillian.checkLogin());
		
	}
	
	@When("I logout trillian")
	public void pressLogout(){
		trillian.logout();
	}
}
